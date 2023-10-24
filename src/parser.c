#include "parser.h" // Token, Parser, Token::*, bool

#include "errors.h"
#include "vec.h"

#include <stdlib.h> // free
#include <limits.h> // INT_MAX

static Token tok_next(Parser *par);

Parser parser_new(Lexer *lex) {
    return (Parser) {
        .lex = lex,
        .cur = T_EOF, // it is never readed
    };
}

static TodoTree *todo_tree() {
    static TodoTree todo = 1;
    return &todo;
};

static TodoTree *parse_error(Parser *par, int err_type, char *msg) {
    printf(
        ":%zu:%zu: error: %s",
        par->lex->token_start.line,
        par->lex->token_start.column,
        msg
    );
    par->error = err_type;
    return NULL;
}

static TodoTree *parse_block(Parser *par, Token end);
static TodoTree *parse_statement(Parser *par);
static TodoTree *parse_if(Parser *par);
static TodoTree *parse_if_expression(Parser *par);
static TodoTree *parse_expression(Parser *par);
static TodoTree *parse_bracket(Parser *par);
static TodoTree *parse_terminal(Parser *par);
static TodoTree *parse_infix(Parser *par, TodoTree *left);
static TodoTree *parse_function_params(Parser *par);
static TodoTree *parse_func_param(Parser *par);
static TodoTree *parse_while(Parser *par);
static TodoTree *parse_decl(Parser *par);
static TodoTree *parse_type(Parser *par);
static TodoTree *parse_func(Parser *par);
static TodoTree *parse_func_decl_param(Parser *par);
static TodoTree *parse_return(Parser *par);

static bool calculate(Vec *ops, Vec *exprs, Vec *fpars);
static int get_precedence(Token op, bool unary);
static Token get_r_version(Token op);
static TodoTree *is_l_operator(Token op);
static size_t op_arg_count(Token op);
static bool is_r_asoc(Token op);

bool parser_parse(Parser *par) {
    TodoTree *res = parse_block(par, T_EOF);
    if (!res) {
        return false;
    }
    // TODO: handle tree
    return true;
}

void parser_free(Parser *p) {
    // TODO: parser_free
}

static Token tok_next(Parser *par) {
    return par->cur = lex_next(par->lex);
}

static TodoTree *parse_block(Parser *par, Token end) {
    tok_next(par); // skip '{'

    TodoTree *res;

    while (par->cur != end) {
        if (par->cur == T_EOF) {
            return parse_error(
                par,
                ERR_SYNTAX,
                "Unexpected eof, expected end of block"
            );
        }
        TodoTree *stmt = parse_statement(par);
        if (!stmt) {
            free(res);
            return NULL;
        }

        // TODO: add stmt to res
        todo_tree(res, stmt);
    }

    return res;
}

static TodoTree *parse_statement(Parser *par) {
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
        return parse_expression(par);
    }
}

static TodoTree *parse_if(Parser *par) {
    tok_next(par); // skip the if

    TodoTree *if_expr = parse_if_expression(par);
    if (!if_expr) {
        return NULL;
    }

    if (par->cur != '{') {
        return parse_error(par, ERR_SYNTAX, "Expected {");
    }

    TodoTree *true_block = parse_block(par, '}');
    if (!true_block) {
        free(if_expr); // TODO: free
        return NULL;
    }

    if (par->cur != T_ELSE) {
        return todo_tree(if_expr, true_block, NULL);
    }

    tok_next(par); // skip the else

    if (par->cur != '{') {
        return parse_error(par, ERR_SYNTAX, "Expected {");
    }

    TodoTree *false_block = parse_block(par, '}');
    if (!false_block) {
        // TODO: free
        free(if_expr);
        free(true_block);
        return NULL;
    }

    return todo_tree(if_expr, true_block, false_block);
}

static TodoTree *parse_if_expression(Parser *par) {
    if (par->cur != T_DECL) {
        return parse_expression(par);
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

    return todo_tree(par->lex->str);
}

static TodoTree *parse_expression(Parser *par) {
    TodoTree *term = NULL;

    switch (par->cur) {
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

static TodoTree *parse_bracket(Parser *par) {
    tok_next(par); // skip '('

    TodoTree *res = parse_expression(par);
    if (!res) {
        return NULL;
    }

    if (par->cur != ')') {
        free(res); // TODO: free
        return parse_error(par, ERR_SYNTAX, "Expected ')'");
    }

    tok_next(par); // skip the ')'
    return res;
}

static TodoTree *parse_terminal(Parser *par) {
    TodoTree *ret;
    switch (par->cur) {
    case T_IDENT:
        ret = todo_tree(par->lex->str);
        break;
    case T_LITERAL:
        ret = todo_tree(par->lex);
        break;
    default:
        return parse_error(par, ERR_SEMANTIC, "Expected terminal");
    }
    tok_next(par);
    return ret;
}

static TodoTree *parse_infix(Parser *par, TodoTree *left) {
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

        bool read_next = true;
        if (op == '(') {
            TodoTree *params = parse_function_params(par);
            if (!params) {
                // TODO: free
                vec_free_with(&fpars, free);
                vec_free_with(&exprs, free);
                vec_free(&ops);
                return NULL;
            }
            read_next = false;
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

static TodoTree *parse_function_params(Parser *par) {
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

static TodoTree *parse_func_param(Parser *par) {
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
        free(&id);
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

    TodoTree **r = vec_pop(&exprs);
    if (!r) {
        return false;
    }

    if (cnt == 1) {
        return todo_tree(*op, *r);
    }

    TodoTree **l = vec_pop(&exprs);
    if (!l) {
        return false;
    }

    return todo_tree(*op, *l, *r);
}

static size_t op_arg_count(Token op) {
    switch (op) {
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
    int prec = 0;
    switch (op) {
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
    switch (op)
    {
    case '+':
        return T_UNARY_PLUS;
    case '-':
        return T_UNARY_MINUS;
    default:
        return op;
    }
}

static TodoTree *is_l_operator(Token op) {
    return op_arg_count(op) == 2 || op == '(';
}

static bool is_r_asoc(Token op) {
    return op == T_DOUBLE_QUES || op == '=';
}

static TodoTree *parse_while(Parser *par) {
    tok_next(par); // skip the while
    TodoTree *cond = parse_expression(par);
    if (!cond) {
        return false;
    }

    if (tok_next(par) != '{') {
        // TODO: free
        free(cond);
        return parse_error(par, ERR_SYNTAX, "Expected '{'");
    }

    TodoTree *loop = parse_block(par, '}');
    if (!loop) {
        // TODO: free
        free(cond);
        free(loop);
        return NULL;
    }

    return todo_tree(cond, loop);
}

static TodoTree *parse_decl(Parser *par) {
    Token dtype = par->lex->subtype;

    if (tok_next(par) != T_IDENT) {
        return NULL;
    }
    String ident = par->lex->str;

    tok_next(par);

    TodoTree *type = NULL;
    if (par->cur == ':') {
        tok_next(par);
        type = parse_type(par);
        if (!type) {
            // TODO: free
            free(&ident);
            return NULL;
        }
        tok_next(par);
    }

    if (par->cur != '=') {
        return todo_tree(dtype, ident, type);
    }

    tok_next(par);
    TodoTree *init = parse_expression(par);
    if (!init) {
        // TODO: free
        free(&ident);
        free(type);
        return NULL;
    }

    return todo_tree(dtype, ident, type, init);
}

static TodoTree *parse_type(Parser *par) {
    if (par->cur != T_TYPE) {
        return parse_error(par, ERR_SYNTAX, "Expected type name");
    }

    bool nillable = false;
    tok_next(par);
    if (par->cur = '?') {
        nillable = true;
        tok_next(par);
    }

    return todo_tree(par->lex->subtype, nillable);
}

static TodoTree *parse_func(Parser *par) {
    tok_next(par); // skip the func
    if (par->cur != T_IDENT) {
        return parse_error(par, ERR_SYNTAX, "expected function identifier");
    }

    String ident = par->lex->str;

    tok_next(par);
    if (par->cur != '(') {
        // TODO: free
        free(&ident);
        return parse_error(
            par,
            ERR_SYNTAX,
            "expected '(' and function parameters"
        );
    }

    Vec params = VEC_NEW(TodoTree *);

    while (tok_next(par) != ')') {
        TodoTree *param = parse_func_decl_param(par);
        if (!param) {
            // TODO: free
            free(&ident);
            vec_free_with(&params, free);
            return NULL;
        }
        VEC_PUSH(&params, TodoTree *, param);
        if (par->cur == ',') {
            if (tok_next(par) == ')') {
                // TODO: free
                free(&ident);
                vec_free_with(&params, free);
                return parse_error(
                    par,
                    ERR_SYNTAX,
                    "Expected next function parameter"
                );
            }
        } else if (par->cur != ')') {
            // TODO: free
            free(&ident);
            vec_free_with(&params, free);
            return parse_error(par, ERR_SYNTAX, "Expected ',', or ')'");
        }
    }

    tok_next(par);

    TodoTree *type = NULL;

    if (par->cur == T_RETURNS) {
        tok_next(par);
        type = parse_type(par);
        if (!type) {
            free(&ident);
            vec_free_with(&params, free);
            return NULL;
        }
    }

    TodoTree *block = parse_block(par, '}');
    if (!block) {
        // TODO: free
        free(&ident);
        vec_free_with(&params, free);
        free(type);
    }

    return todo_tree(ident, params, type, block);
}

static TodoTree *parse_func_decl_param(Parser *par) {
    String name = NUL_STR;

    if (par->cur != '_' && par->cur != T_IDENT) {
        return parse_error(par, ERR_SYNTAX, "Expected parameter name");
    } else if (par->cur == T_IDENT) {
        name = par->lex->str;
    }

    if (tok_next(par) != T_IDENT) {
        // TODO: free
        free(&name);
        return parse_error(par, ERR_SYNTAX, "Expected parameter identifier");
    }
    String ident = par->lex->str;

    if (tok_next(par) != ':') {
        // TODO: free
        free(&name);
        free(&ident);
        return parse_error(par, ERR_SYNTAX, "Expected ':'");
    }

    tok_next(par);

    TodoTree *type = parse_type(par);
    if (!type) {
        // TODO: free
        free(&name);
        free(&ident);
        return NULL;
    }

    return todo_tree(name, ident, type);
}

static TodoTree *parse_return(Parser *par) {
    tok_next(par);
    TodoTree *expr = parse_expression(par);
    if (!expr) {
        return NULL;
    }
    return todo_tree(par);
}
