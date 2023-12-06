/**
 * IFJ23
 *
 * xstigl00 Jakub Antonín Štigler
 */

#ifndef PRINTER_H_INCLUDED
#define PRINTER_H_INCLUDED

#include "enums.h"
#include "ast.h"
#include "lexer.h"

void print_data_type(DataType type);
void print_token(Token token);
void print_ast_block(AstBlock *block, size_t depth);
void print_ast_binary_op(AstBinaryOp *op, size_t depth);
void print_ast_unary_op(AstUnaryOp *op, size_t depth);
void print_ast_literal(AstLiteral *l);
void print_ast_func_call_param(AstFuncCallParam *par, size_t depth);
void print_ast_function_call(AstFunctionCall *call, size_t depth);
void print_ast_function_decl(AstFunctionDecl *decl, size_t depth);
void print_ast_return(AstReturn *ret, size_t depth);
void print_ast_variable_decl(AstVariableDecl *decl, size_t depth);
void print_ast_condition(AstCondition *cond, size_t depth);
void print_ast_if(AstIf *if_v, size_t depth);
void print_ast_while(AstWhile *while_v, size_t depth);
void print_ast_variable(AstVariable *var);
void print_ast_expr(AstExpr *expr, size_t depth);
void print_ast_stmt(AstStmt *stmt, size_t depth);

#endif // PRINTER_H_INCLUDED
