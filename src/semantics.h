/**
 * IFJ23
 *
 * xdanie14 Tomáš Daniel
 * xstigl00 Jakub Antonín Štigler
 */

#ifndef SEMANTICS_H_INCLUDED
#define SEMANTICS_H_INCLUDED

#include "stdbool.h"
#include "ast.h"
#include "errors.h"
#include "vec.h"
#include "utils.h"
#include "infix_parser.h"

typedef struct {
    bool in_func;
    bool ret_stmt_found;
    bool in_func_call;
    DataType func_ret_type;
} Context;

/////////////////////////////////////////////////////////////////////////
bool sem_func_param(
    String label,
    SymItem *ident,
    DataType type,
    FuncParam *res
);

AstBlock *sem_block(FilePos pos, Vec stmts, bool top_level);

AstCondition *sem_expr_condition(AstExpr *expr);
AstCondition *sem_let_condition(FilePos pos, SymItem *ident);

AstStmt *sem_expr_stmt(AstExpr *expr);
AstStmt *sem_while(FilePos pos, AstCondition *cond, AstBlock *loop);
AstStmt *sem_var_decl(FilePos pos, SymItem *ident, AstExpr *expr);
AstStmt *sem_func_decl(
    FilePos pos,
    SymItem *ident,
    AstBlock *body
);
AstStmt *sem_if(
    FilePos pos,
    AstCondition *cond,
    AstBlock *true_block,
    AstBlock *false_block
);
AstStmt *sem_return(FilePos pos, AstExpr *expr);

AstExpr *sem_unary(FilePos pos, AstExpr *expr, Token op);
AstExpr *sem_variable(FilePos pos, SymItem *ident);
AstExpr *sem_literal(FilePos pos, FullToken token);
AstExpr *sem_call(FilePos pos, AstExpr *calle, Vec params);
bool sem_lex_literal(Lexer *lex, AstLiteral *res);
AstExpr *sem_binary(FilePos pos, AstExpr *left, Token op, AstExpr *right);
void sem_if_block_end(AstCondition *cond);
/////////////////////////////////////////////////////////////////////////

#endif // SEMANTICS_H_INCLUDED
