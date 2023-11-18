#include "parser.h" // Token, Parser, Token::*, bool

#include "errors.h"
#include "vec.h"
#include "ast.h"
#include "semantics.h"

#include <stdlib.h> // free
#include <limits.h> // INT_MAX

// TODO: remove, it is here only to show errors when it is used
#define todo_tree(...) {+++}

static Token tok_next(Parser *par);

Parser parser_new(Lexer *lex, Symtable *table) {
    return (Parser) {
        .lex = lex,
        .cur = T_EOF, // it is never readed
        .table = table,
    };
}

static void *parse_error(Parser *par, int err_type, char *msg) {
    printf(
        ":%zu:%zu: error: %s",
        par->lex->token_start.line,
        par->lex->token_start.column,
        msg
    );
    par->error = err_type;
    return NULL;
}

static AstBlock *parse_block(Parser *par, bool top_level);
static AstStmt *parse_statement(Parser *par);
static AstStmt *parse_if(Parser *par);
static AstCondition *parse_if_condition(Parser *par);
static AstExpr *parse_expression(Parser *par);
static AstExpr *parse_bracket(Parser *par);
static AstExpr *parse_terminal(Parser *par);
static AstExpr *parse_infix(Parser *par, AstExpr *left);
static bool parse_function_params(Parser *par, Vec *res);
static AstFuncCallParam *parse_func_param(Parser *par);
static AstStmt *parse_while(Parser *par);
static AstStmt *parse_decl(Parser *par);
static bool parse_type(Parser *par, DataType *res);
static AstStmt *parse_func(Parser *par);
static bool parse_func_decl_param(Parser *par, FuncParam *res);
static AstStmt *parse_return(Parser *par);

static bool calculate(Vec *ops, Vec *exprs, Vec *fpars);
static int get_precedence(Token op, bool unary);
static Token get_r_version(Token op);
static bool is_l_operator(Token op);
static size_t op_arg_count(Token op);
static bool is_r_asoc(Token op);

AstBlock *parser_parse(Parser *par) {
    return parse_block(par, true);
}

void parser_free(Parser *p) {
    // TODO: parser_free
}

static Token tok_next(Parser *par) {
    return par->cur = lex_next(par->lex);
}

static AstBlock *parse_block(Parser *par, bool top_level) {
    sym_scope_add(par->table);

    tok_next(par); // skip '{'

    Token end = top_level ? T_EOF : '}';

    Vec stmts = VEC_NEW(AstStmt *);

    while (par->cur != end) {
        if (par->cur == T_EOF) {
            sym_scope_pop(par->table);
            return parse_error(
                par,
                ERR_SYNTAX,
                "Unexpected eof, expected end of block"
            );
        }
        AstStmt *stmt = parse_statement(par);
        if (!stmt) {
            sym_scope_pop(par->table);
            vec_free_with(&stmts, (FreeFun)ast_free_stmt);
            return NULL;
        }

        VEC_PUSH(&stmts, AstStmt *, stmt);
    }

    sym_scope_pop(par->table);
    return sem_block(stmts, top_level);
}

static AstStmt *parse_statement(Parser *par) {
    switch (par->cur) {
    case T_IF:
        return parse_if(par);
    case T_WHILE:
        return parse_while(par);
    case T_DECL:
        return parse_decl(par);
    case T_FUNC:
        return parse_func(par);
    case T_RETURN:
        return parse_return(par);
    default:
        return sem_expr_stmt(parse_expression(par));
    }
}

static AstStmt *parse_if(Parser *par) {
    tok_next(par); // skip the if

    AstCondition *cond = parse_if_condition(par);
    if (!cond) {
        return NULL;
    }

    if (par->cur != '{') {
        return parse_error(par, ERR_SYNTAX, "Expected {");
    }

    AstBlock *true_block = parse_block(par, '}');
    if (!true_block) {
        ast_free_condition(&cond);
        return NULL;
    }

    if (par->cur != T_ELSE) {
        return sem_if(cond, true_block, NULL);
    }

    tok_next(par); // skip the else

    if (par->cur != '{') {
        return parse_error(par, ERR_SYNTAX, "Expected {");
    }

    AstBlock *false_block = parse_block(par, '}');
    if (!false_block) {
        ast_free_condition(&cond);
        ast_free_block(&false_block);
        return NULL;
    }

    return sem_if(cond, true_block, false_block);
}

static AstCondition *parse_if_condition(Parser *par) {
    if (par->cur != T_DECL) {
        return sem_expr_condition(parse_expression(par));
    }

    if (par->lex->subtype != TD_LET) {
        return parse_error(
            par,
            ERR_SYNTAX,
            "You cannot use 'var' in if, use 'let'"
        );
    }

    if (tok_next(par) != T_IDENT) {
        return parse_error(
            par,
            ERR_SYNTAX,
            "Expected identifier after let condition"
        );
    }

    SymItem *ident = sym_find(par->table, par->lex->str);

    return sem_let_condition(ident);
}

static AstExpr *parse_expression(Parser *par) {
    AstExpr *term = NULL;

    switch ((int)par->cur) {
    case '(':
        term = parse_bracket(par);
        break;
    case '-':
    case '+':
    case '!':
        break;
    default:
        term = parse_terminal(par);
        if (!term) {
            return parse_error(par, ERR_SYNTAX, "Expected expression");
        }
        break;
    }

    return parse_infix(par, term);
}

static AstExpr *parse_bracket(Parser *par) {
    tok_next(par); // skip '('

    AstExpr *res = parse_expression(par);
    if (!res) {
        return NULL;
    }

    if (par->cur != ')') {
        ast_free_expr(&res);
        return parse_error(par, ERR_SYNTAX, "Expected ')'");
    }

    tok_next(par); // skip the ')'
    return res;
}

static AstExpr *parse_terminal(Parser *par) {
    AstExpr *ret;
    SymItem *ident;
    switch (par->cur) {
    case T_IDENT:
        ret = sem_lex_variable(par->lex);
        break;
    case T_LITERAL:
        ret = sem_lex_literal(par->lex);
        break;
    default:
        return parse_error(par, ERR_SEMANTIC, "Expected terminal");
    }
    tok_next(par);
    return ret;
}

// TODO: parse infix
static AstExpr *parse_infix(Parser *par, AstExpr *left) {
    Vec exprs = VEC_NEW(TodoTree *);
    Vec ops = VEC_NEW(Token); // operators, 0 is brackets as exprs
    Vec fpars = VEC_NEW(TodoTree *); // function parameters for function calls

    bool no_l = true; // when true, the next value cannot take from left
    bool do_l = false; // when true, the next operator must take from left

    if (left) {
        VEC_PUSH(&exprs, TodoTree *, left);
        no_l = false;
        do_l = true;
    }

    while (true) {
        // 0 when not operator
        Token op = par->cur;
        int prec = get_precedence(op, no_l);

        if (prec == 0) {
            // not operator
            if (do_l) {
                // expected operator => end of expression
                break;
            }

            if (par->cur != T_LITERAL && par->cur != T_IDENT) {
                break;
            }

            TodoTree *term = parse_terminal(par);
            if (!term) {
                return NULL;
            }

            do_l = true;
            no_l = false;
            VEC_PUSH(&exprs, TodoTree *, term);
            continue;
        }

        if (no_l) {
            // don't expect operator
            if (op == '(') {
                VEC_PUSH(&ops, Token, T_EXPR_PAREN);
                do_l = false;
                no_l = true;
                continue;
            }

            if (op == ')') {
                Token *last = vec_last(&ops);
                while (last && *last != T_EXPR_PAREN) {
                    if (!calculate(&ops, &exprs, &fpars)) {
                        return NULL;
                    }
                }
                if (last) { // pop the T_EXPR_PAREN
                    vec_pop(&ops);
                }
            }

            Token op = get_r_version(op);
            if (op == T_ERR) {
                return parse_error(
                    par,
                    ERR_SYNTAX,
                    "Operator has no left argument"
                );
            }

            VEC_PUSH(&ops, Token, op);
            no_l = false;
            do_l = true;
            continue;
        }

        if (do_l && !is_l_operator(op)) {
            break;
        }

        if (op == '(') {
            TodoTree *params = parse_function_params(par);
            if (!params) {
                // TODO: free
                vec_free_with(&fpars, free);
                vec_free_with(&exprs, free);
                vec_free(&ops);
                return NULL;
            }
            VEC_PUSH(&fpars, TodoTree *, params);
        }

        while (true) {
            Token *other_op = vec_last(&ops);
            int other_prec = INT_MAX;
            if (other_op) {
                other_prec = get_precedence(*other_op, false);
            }

            if (prec < other_prec || (prec == other_prec && is_r_asoc(op))) {
                VEC_PUSH(&ops, Token, op);
                break;
            }

            if (!calculate(&ops, &exprs, &fpars)) {
                // TODO: free
                vec_free_with(&fpars, free);
                vec_free_with(&exprs, free);
                vec_free(&ops);
                return NULL;
            }
        }
    }

    if (exprs.len != 1 || ops.len != 0 || fpars.len != 0) {
        vec_free_with(&fpars, free);
        vec_free_with(&exprs, free);
        vec_free(&ops);
        return parse_error(par, ERR_SYNTAX, "Unexpected end of expression");
    }

    TodoTree *ret = VEC_LAST(&exprs, TodoTree *);
    vec_free(&fpars);
    vec_free(&exprs);
    vec_free(&ops);
    return ret;
}

static bool parse_function_params(Parser *par, Vec *res) {
    tok_next(par);
    Vec params = VEC_NEW(TodoTree *);
    while (par->cur != ')') {
        TodoTree *param = parse_func_param(par);
        if (!param) {
            // TODO: free
            vec_free_with(&params, free);
            return NULL;
        }
        VEC_PUSH(&params, TodoTree *, param);
        if (par->cur == ',') {
            if (tok_next(par) == ')') {
                // TODO: free
                vec_free_with(&params, free);
                return parse_error(par, ERR_SYNTAX, "Expected next argument");
            }
        } else if (par->cur != ')') {
            // TODO: free
            vec_free_with(&params, free);
            return parse_error(par, ERR_SYNTAX, "Expected ',' or ')'");
        }
    }
    tok_next(par);
    return todo_tree(params);
}

static AstFuncCallParam *parse_func_param(Parser *par) {
    if (par->cur != T_IDENT) {
        return NULL;
    }

    // TODO: id
    String id = par->lex->str;

    if (tok_next(par) != ':') {
        return todo_tree(id);
    }

    if (tok_next(par) != T_IDENT) {
        // TODO: free
        str_free(&id);
        return NULL;
    }

    return todo_tree(id, par->lex->str);
}

static bool calculate(Vec *ops, Vec *exprs, Vec *fpars) {
    Token *op = vec_pop(ops);
    if (!op || *op == T_EXPR_PAREN) {
        return false;
    }

    size_t cnt = op_arg_count(*op);
    if (cnt == 0) {
        return false;
    }

    TodoTree **r = vec_pop(exprs);
    if (!r) {
        return false;
    }

    if (cnt == 1) {
        return todo_tree(*op, *r);
    }

    TodoTree **l = vec_pop(exprs);
    if (!l) {
        return false;
    }

    return todo_tree(*op, *l, *r);
}

static size_t op_arg_count(Token op) {
    switch ((int)op) {
    case '(':
    case '!':
    case T_UNARY_MINUS:
    case T_UNARY_PLUS:
        return 1;

    case '*':
    case '/':
    case '+':
    case '-':
    case '<':
    case '>':
    case '=':
    case T_EQUALS:
    case T_DIFFERS:
    case T_LESS_OR_EQUAL:
    case T_GREATER_OR_EQUAL:
    case T_DOUBLE_QUES:
        return 2;

    default:
        return 0;
    }
}

// +------------+-----------------+-------------+---------------+
// | PRECEDENCE | OPERATOR        | DESCRIPTION | ASOCIATIVITY  |
// +------------+-----------------+-------------+---------------+
// | 1          | ()              | call        | left to right |
// +------------+-----------------+-------------+               |
// | 2          | ! - +           | prefix      |               |
// +------------+-----------------+-------------+               |
// | 3          | * /             | mul/div     |               |
// +------------+-----------------+-------------+               |
// | 4          | + -             | add/sub     |               |
// +------------+-----------------+-------------+               |
// | 5          | == != < > <= >= | comparison  |               |
// +------------+-----------------+-------------+---------------+
// | 6          | ??              | nil check   | right to left |
// +------------+-----------------+-------------+               |
// | 7          | =               | assignment  |               |
// +------------+-----------------+-------------+---------------+

static int get_precedence(Token op, bool unary) {
    switch ((int)op) {
    case '(':
        return 1;

    case '!':
    case T_UNARY_MINUS:
    case T_UNARY_PLUS:
        return 2;

    case '*':
    case '/':
        return 3;

    case '+':
    case '-':
        if (unary) {
            return 2;
        }
        return 4;

    case '<':
    case '>':
    case T_EQUALS:
    case T_DIFFERS:
    case T_LESS_OR_EQUAL:
    case T_GREATER_OR_EQUAL:
        return 5;

    case T_DOUBLE_QUES:
        return 6;

    case '=':
        return 7;

    default:
        return INT_MAX;
    }
}

static Token get_r_version(Token op) {
    switch ((int)op)
    {
    case '+':
        return T_UNARY_PLUS;
    case '-':
        return T_UNARY_MINUS;
    default:
        return op;
    }
}

static bool is_l_operator(Token op) {
    return op_arg_count(op) == 2 || op == '(';
}

static bool is_r_asoc(Token op) {
    return op == T_DOUBLE_QUES || op == '=';
}

static AstStmt *parse_while(Parser *par) {
    tok_next(par); // skip the while
    AstExpr *cond = parse_expression(par);
    if (!cond) {
        return false;
    }

    if (tok_next(par) != '{') {
        ast_free_expr(&cond);
        return parse_error(par, ERR_SYNTAX, "Expected '{'");
    }

    AstBlock *loop = parse_block(par, '}');
    if (!loop) {
        ast_free_expr(&cond);
        return NULL;
    }

    return sem_while(cond, loop);
}

static AstStmt *parse_decl(Parser *par) {
    bool mutable = par->lex->subtype == TD_VAR;

    if (tok_next(par) != T_IDENT) {
        return NULL;
    }

    SymItem *ident = sym_declare(par->table, par->lex->str, false);
    if (!ident) {
        return NULL;
    }

    tok_next(par);

    DataType type = DT_NONE;
    if (par->cur == ':') {
        tok_next(par);
        if (!parse_type(par, &type)) {
            return NULL;
        }
        tok_next(par);
    }

    if (par->cur != '=') {
        return sem_var_decl(mutable, ident, type, NULL);
    }

    tok_next(par);
    AstExpr *init = parse_expression(par);
    if (!init) {
        return NULL;
    }

    return sem_var_decl(mutable, ident, type, init);
}

static bool parse_type(Parser *par, DataType *res) {
    if (par->cur != T_TYPE) {
        parse_error(par, ERR_SYNTAX, "Expected type name");
        return false;
    }

    *res = par->lex->subtype;

    tok_next(par);
    if (par->cur == '?') {
        *res |= DT_NIL;
    }

    return true;
}

static AstStmt *parse_func(Parser *par) {
    tok_next(par); // skip the func
    if (par->cur != T_IDENT) {
        return parse_error(par, ERR_SYNTAX, "expected function identifier");
    }

    SymItem *ident = sym_declare(par->table, par->lex->str, true);

    tok_next(par);
    if (par->cur != '(') {
        return parse_error(
            par,
            ERR_SYNTAX,
            "expected '(' and function parameters"
        );
    }

    Vec params = VEC_NEW(FuncParam);
    sym_scope_add(par->table);

    while (tok_next(par) != ')') {
        FuncParam param;
        if (!parse_func_decl_param(par, &param)) {
            sym_scope_pop(par->table);
            vec_free_with(&params, (FreeFun)sym_free_func_param);
            return NULL;
        }
        VEC_PUSH(&params, FuncParam, param);
        if (par->cur == ',') {
            if (tok_next(par) == ')') {
                sym_scope_pop(par->table);
                vec_free_with(&params, (FreeFun)sym_free_func_param);
                return parse_error(
                    par,
                    ERR_SYNTAX,
                    "Expected next function parameter"
                );
            }
        } else if (par->cur != ')') {
            sym_scope_pop(par->table);
            vec_free_with(&params, (FreeFun)sym_free_func_param);
            return parse_error(par, ERR_SYNTAX, "Expected ',', or ')'");
        }
    }

    tok_next(par);

    DataType type = DT_NONE;

    if (par->cur == T_RETURNS) {
        tok_next(par);
        if (!parse_type(par, &type)) {
            sym_scope_pop(par->table);
            vec_free_with(&params, (FreeFun)sym_free_func_param);
            return NULL;
        }
    }

    AstBlock *block = parse_block(par, '}');
    if (!block) {
        sym_scope_pop(par->table);
        vec_free_with(&params, (FreeFun)sym_free_func_param);
        return NULL;
    }

    sym_scope_pop(par->table);

    return sem_func_decl(ident, params, type, block);
}

static bool parse_func_decl_param(Parser *par, FuncParam *res) {
    String label = NUL_STR;

    if (par->cur != '_' && par->cur != T_IDENT) {
        parse_error(par, ERR_SYNTAX, "Expected parameter name");
        return false;
    } else if (par->cur == T_IDENT) {
        label = str_clone(par->lex->str);
    }

    if (tok_next(par) != T_IDENT) {
        str_free(&label);
        parse_error(par, ERR_SYNTAX, "Expected parameter identifier");
        return false;
    }

    SymItem *ident = sym_declare(par->table, par->lex->str, false);

    if (tok_next(par) != ':') {
        str_free(&label);
        parse_error(par, ERR_SYNTAX, "Expected ':'");
        return false;
    }

    tok_next(par);

    DataType type;
    if (!parse_type(par, &type)) {
        str_free(&label);
        return NULL;
    }

    return sem_func_param(label, ident, type, &res);
}

static AstStmt *parse_return(Parser *par) {
    tok_next(par);
    AstExpr *expr = parse_expression(par);
    if (!expr) {
        return NULL;
    }
    return sem_return(expr);
}
