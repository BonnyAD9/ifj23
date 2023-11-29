#ifndef SEMANTICS_H_INCLUDED
#define SEMANTICS_H_INCLUDED

#include "stdbool.h"
#include "ast.h"
#include "errors.h"
#include "vec.h"
#include "utils.h"
#include "infix_parser.h"

/////////////////////////////////////////////////////////////////////////
bool         sem_func_param(String label, SymItem *ident, DataType type, FuncParam *res);

AstBlock     *sem_block(Vec stmts, bool top_level);

AstCondition *sem_expr_condition(AstExpr *expr);
AstCondition *sem_let_condition(SymItem *ident);

AstStmt      *sem_expr_stmt(AstExpr *expr);
AstStmt      *sem_while(AstCondition *cond, AstBlock *loop);
AstStmt      *sem_var_decl(SymItem *ident, AstExpr *expr);
AstStmt      *sem_func_decl(SymItem *ident, Vec params, DataType return_type, AstBlock *body);
AstStmt      *sem_if(AstCondition *cond, AstBlock *true_block, AstBlock *false_block);
AstStmt      *sem_return(AstExpr *expr);

AstExpr      *sem_unary(AstExpr *expr, Token op);
AstExpr      *sem_variable(SymItem *ident);
AstExpr      *sem_literal(FullToken token);
AstExpr      *sem_call(AstExpr *calle, Vec params);
bool         sem_lex_literal(Lexer *lex, AstLiteral *res);
AstExpr      *sem_binary(AstExpr *left, Token op, AstExpr *right);
/////////////////////////////////////////////////////////////////////////

#endif // SEMANTICS_H_INCLUDED
