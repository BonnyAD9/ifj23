/**
 * IFJ23
 *
 * xdanie14 Tomáš Daniel
 * xstigl00 Jakub Antonín Štigler
 */

#include "parser.h" // Token, Parser, Token::*, bool

#include "errors.h"
#include "vec.h"
#include "ast.h"
#include "semantics.h"
#include "infix_parser.h"
#include "debug_tools.h"

#include <stdlib.h> // free
#include <limits.h> // INT_MAX

// TODO: remove, it is here only to show errors when it is used
#define todo_tree(...) {+++}

Token tok_next(Parser *par);

Parser parser_new(Lexer *lex, Symtable *table) {
    return (Parser) {
        .lex = lex,
        .cur = T_EOF, // it is never readed
        .table = table,
    };
}

void *parse_error(Parser *par, int err_type, char *msg) {
    printf(
        DEBUG_FILE ":%zu:%zu: error: %s\n",
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
static AstCondition *parse_condition(Parser *par);
static AstExpr *parse_expression(Parser *par);
bool parse_func_params(Parser *par, Vec *res);
static bool parse_func_param(Parser *par, AstFuncCallParam *res);
static AstStmt *parse_while(Parser *par);
static AstStmt *parse_decl(Parser *par);
static bool parse_type(Parser *par, DataType *res);
static AstStmt *parse_func(Parser *par);
static bool parse_func_decl_param(Parser *par, FuncParam *res);
static AstStmt *parse_return(Parser *par);

static bool is_expr_token(Token t);

AstBlock *parser_parse(Parser *par) {
    return parse_block(par, true);
}

void parser_free(Parser *p) { } // current implementation of parser has
                                // nothing to free

Token tok_next(Parser *par) {
    return par->cur = lex_next(par->lex);
}

static AstBlock *parse_block(Parser *par, bool top_level) {
    if (!top_level) {
        sym_scope_add(par->table);
    }

    FilePos pos = par->lex->token_start;
    tok_next(par); // skip '{'

    Token end = top_level ? T_EOF : '}';

    Vec stmts = VEC_NEW(AstStmt *);

    while (par->cur != end) {
        if (par->cur == T_EOF) {
            if (!top_level) {
                sym_scope_pop(par->table);
            }
            vec_free_with(&stmts, (FreeFun)ast_free_stmt);
            return parse_error(
                par,
                ERR_SYNTAX,
                "Unexpected eof, expected end of block"
            );
        }
        AstStmt *stmt = parse_statement(par);
        if (!stmt) {
            if (!top_level) {
                sym_scope_pop(par->table);
            }
            vec_free_with(&stmts, (FreeFun)ast_free_stmt);
            return NULL;
        }

        if (!vec_push(&stmts, &stmt)) {
            if (!top_level) {
                sym_scope_pop(par->table);
            }
            vec_free_with(&stmts, (FreeFun)ast_free_stmt);
            return NULL;
        }
    }

    if (!top_level) {
        tok_next(par); // skip the '}'
        sym_scope_pop(par->table);
    }

    return sem_block(pos, stmts, top_level);
}

static AstStmt *parse_statement(Parser *par) {
    AstExpr *expr = NULL;
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
        expr = parse_expression(par);
        return expr ? sem_expr_stmt(expr) : NULL;
    }
}

static AstStmt *parse_if(Parser *par) {
    FilePos pos = par->lex->token_start;
    tok_next(par); // skip the if

    AstCondition *cond = parse_condition(par);
    if (!cond) {
        return NULL;
    }

    if (par->cur != '{') {
        ast_free_condition(&cond);
        return parse_error(par, ERR_SYNTAX, "Expected {");
    }

    AstBlock *true_block = parse_block(par, false);
    if (!true_block) {
        ast_free_condition(&cond);
        return NULL;
    }

    sem_if_block_end(cond);

    if (par->cur != T_ELSE) {
        return sem_if(pos, cond, true_block, NULL);
    }

    tok_next(par); // skip the else

    if (par->cur != '{') {
        ast_free_condition(&cond);
        ast_free_block(&true_block);
        return parse_error(par, ERR_SYNTAX, "Expected {");
    }

    AstBlock *false_block = parse_block(par, false);
    if (!false_block) {
        ast_free_condition(&cond);
        ast_free_block(&true_block);
        return NULL;
    }

    return sem_if(pos, cond, true_block, false_block);
}

static AstCondition *parse_condition(Parser *par) {
    if (par->cur != T_DECL) {
        AstExpr *expr = parse_expression(par);
        if (!expr) {
            return NULL;
        }
        return sem_expr_condition(expr);
    }

    if (par->lex->subtype != TD_LET) {
        return parse_error(
            par,
            ERR_SYNTAX,
            "You cannot use 'var' in if, use 'let'"
        );
    }

    FilePos pos = par->lex->token_start;

    if (tok_next(par) != T_IDENT) {
        return parse_error(
            par,
            ERR_SYNTAX,
            "Expected identifier after let condition"
        );
    }

    SymItem *ident = sym_find(par->table, par->lex->str);

    tok_next(par);

    return sem_let_condition(pos, ident);
}

static AstExpr *parse_expression(Parser *par) {
    return parse_infix(par);
}

bool parse_func_params(Parser *par, Vec *res) {
    tok_next(par);
    while (par->cur != ')') {
        FilePos pos = par->lex->token_start;
        AstFuncCallParam param;
        if (!parse_func_param(par, &param)) {
            return false;
        }
        param.pos = pos;
        if (!vec_push(res, &param)) {
            return OTHER_ERR_FALSE;
        }
        if (par->cur != ',' && par->cur != ')') {
            parse_error(par, ERR_SYNTAX, "Expected ',' or ')'");
            return false;
        }
        if (par->cur == ',') {
            tok_next(par);
        }
    }
    tok_next(par);
    return true;
}

static bool parse_func_param(Parser *par, AstFuncCallParam *res) {
    if (par->cur != T_IDENT && par->cur != T_LITERAL) {
        parse_error(par, ERR_SYNTAX, "Expected identifier or literal.");
        return false;
    }

    res->name = NUL_STR;

    if (par->cur == T_LITERAL) {
        res->type = AST_LITERAL;
        if (!sem_lex_literal(par->lex, &res->literal)) {
            return false;
        };
        tok_next(par);
        return true;
    }

    String name = str_clone(par->lex->str);
    CHECK(name.str);

    tok_next(par);
    if (par->cur != ':') {
        res->type = AST_VARIABLE;
        SymItem *ident = sym_find(par->table, name);
        str_free(&name);
        if (!ident) {
            return false;
        }
        res->variable = ident;
        return true;
    }

    tok_next(par);
    if (par->cur == T_LITERAL) {
        res->name = name;
        res->type = AST_LITERAL;
        if (!sem_lex_literal(par->lex, &res->literal)) {
            str_free(&name);
            return false;
        };
        tok_next(par);
        return true;
    }

    if (par->cur != T_IDENT) {
        str_free(&name);
        parse_error(par, ERR_SYNTAX, "Expected variable or literal");
        return false;
    }

    res->name = name;
    res->type = AST_VARIABLE;
    SymItem *ident = sym_find(par->table, par->lex->str);
    if (!ident) {
        str_free(&res->name);
        return false;
    }
    res->variable = ident;
    tok_next(par);
    return true;
}

static AstStmt *parse_while(Parser *par) {
    FilePos pos = par->lex->token_start;
    tok_next(par); // skip the while

    AstCondition *cond = parse_condition(par);
    if (!cond) {
        return false;
    }

    if (par->cur != '{') {
        ast_free_condition(&cond);
        return parse_error(par, ERR_SYNTAX, "Expected '{'");
    }

    AstBlock *loop = parse_block(par, false);
    if (!loop) {
        ast_free_condition(&cond);
        return NULL;
    }
    sem_if_block_end(cond);

    return sem_while(pos, cond, loop);
}

static AstStmt *parse_decl(Parser *par) {
    bool mutable = par->lex->subtype == TD_VAR;
    FilePos start_pos = par->lex->token_start;

    if (tok_next(par) != T_IDENT) {
        parse_error(par, ERR_SYNTAX, "Missing ident");
        return NULL;
    }

    String name = str_clone(par->lex->str);
    CHECK(name.str);
    FilePos id_pos = par->lex->token_start;

    tok_next(par);

    DataType type = DT_NONE;
    if (par->cur == ':') {
        tok_next(par);
        if (!parse_type(par, &type)) {
            str_free(&name);
            return NULL;
        }
    }

    AstExpr *init = NULL;
    if (par->cur == '=') {
        tok_next(par);
        init = parse_expression(par);
        if (!init) {
            str_free(&name);
            return NULL;
        }
    }

    SymItem *ident = sym_declare(par->table, name, false);
    str_free(&name);
    if (!ident) {
        ast_free_expr(&init);
        return NULL;
    }

    ident->file_pos = id_pos;
    sym_item_var(ident, sym_var_new(type, mutable));

    return sem_var_decl(start_pos, ident, init);
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
        tok_next(par);
    }

    return true;
}

static AstStmt *parse_func(Parser *par) {
    FilePos pos = par->lex->token_start;
    tok_next(par); // skip the func

    if (par->cur != T_IDENT) {
        return parse_error(par, ERR_SYNTAX, "expected function identifier");
    }

    SymItem *ident = sym_declare(par->table, par->lex->str, true);
    if (!ident) {
        return NULL;
    }

    ident->file_pos = par->lex->token_start;
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
    tok_next(par);

    while (par->cur != ')') {
        FuncParam param;
        if (!parse_func_decl_param(par, &param)) {
            sym_scope_pop(par->table);
            vec_free_with(&params, (FreeFun)sym_free_func_param);
            return NULL;
        }
        if (!vec_push(&params, &param)) {
            sym_scope_pop(par->table);
            vec_free_with(&params, (FreeFun)sym_free_func_param);
            sym_free_func_param(&param);
            return NULL;
        }
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

    DataType type = DT_VOID;

    if (par->cur == T_RETURNS) {
        tok_next(par);
        if (!parse_type(par, &type)) {
            sym_scope_pop(par->table);
            vec_free_with(&params, (FreeFun)sym_free_func_param);
            return NULL;
        }
    }

    sym_item_func(ident, sym_func_new(type, params));

    AstBlock *block = parse_block(par, false);
    if (!block) {
        sym_scope_pop(par->table);
        return NULL;
    }

    sym_scope_pop(par->table);

    return sem_func_decl(pos, ident, block);
}

static bool parse_func_decl_param(Parser *par, FuncParam *res) {
    String label = NUL_STR;

    if (par->cur != '_' && par->cur != T_IDENT) {
        parse_error(par, ERR_SYNTAX, "Expected parameter name");
        return false;
    } else if (par->cur == T_IDENT) {
        label = str_clone(par->lex->str);
        CHECK(label.str);
    }

    if (tok_next(par) != T_IDENT && par->cur != '_') {
        str_free(&label);
        parse_error(par, ERR_SYNTAX, "Expected parameter identifier");
        return false;
    }

    SymItem *ident = par->cur == '_'
        ? sym_declare(par->table, STR("_"), false)
        : sym_declare(par->table, par->lex->str, false);

    if (!ident) {
        str_free(&label);
        return false;
    }

    // Make sure func params are always initialized
    ident->var.initialized = true;

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

    return sem_func_param(label, ident, type, res);
}

static AstStmt *parse_return(Parser *par) {
    FilePos pos = par->lex->token_start;
    tok_next(par);

    AstExpr *expr = NULL;

    if (is_expr_token(par->cur)) {
        expr = parse_expression(par);
        if (!expr) {
            return NULL;
        }
    }

    return sem_return(pos, expr);
}

static bool is_expr_token(Token t) {
    return t == T_LITERAL || t == T_IDENT || t == '+' || t == '(' || t == '-';
}
