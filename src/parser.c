#include "parser.h" // Token, Parser, Token::*, bool

#include "errors.h"
#include "vec.h"
#include "ast.h"
#include "semantics.h"
#include "infix_parser.h"

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
static bool parse_literal(Parser *par, AstLiteral *res);
bool parse_func_params(Parser *par, Vec *res);
static bool parse_func_param(Parser *par, AstFuncCallParam *res);
static AstStmt *parse_while(Parser *par);
static AstStmt *parse_decl(Parser *par);
static bool parse_type(Parser *par, DataType *res);
static AstStmt *parse_func(Parser *par);
static bool parse_func_decl_param(Parser *par, FuncParam *res);
static AstStmt *parse_return(Parser *par);

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
    return parse_infix(par);
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

static bool parse_literal(Parser *par, AstLiteral *res) {

}

bool parse_func_params(Parser *par, Vec *res) {
    tok_next(par);
    while (par->cur != ')') {
        AstFuncCallParam param;
        if (!parse_func_param(par, &param)) {
            return false;
        }
        if (par->cur != ',' && par->cur != ')') {
            parse_error(par, ERR_SYNTAX, "Expected ',' or ')'");
            return false;
        }
        tok_next(par);
    }
    tok_next(par);
    return true;
}

static bool parse_func_param(Parser *par, AstFuncCallParam *res) {
    if (par->cur != T_IDENT && par->cur != T_LITERAL) {
        return false;
    }

    res->name = NUL_STR;

    if (par->cur == T_LITERAL) {
        res->type = AST_LITERAL;
        if (!parse_literal(par, &res->literal)) {
            return false;
        };
        tok_next(par);
        return true;
    }

    String name = str_clone(par->lex->str);

    tok_next(par);
    if (par->cur == T_LITERAL) {
        res->name = name;
        res->type = AST_LITERAL;
        if (!parse_literal(par, &res->literal)) {
            str_free(&name);
            return false;
        };
        tok_next(par);
    }

    if (par->cur == T_IDENT) {
        res->name = name;
        res->type = AST_VARIABLE;
        // TODO: parse variable exit
    }

    // TODO: name is variable
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
