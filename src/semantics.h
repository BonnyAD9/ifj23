#ifndef SEMANTICS_H_INCLUDED
#define SEMANTICS_H_INCLUDED

#include <stdbool.h> // bool

#include "ast.h"
#include "enums.h"
#include "symtable.h"

bool check_statement(AstStmt *stmt);

typedef struct {
    bool in_func;
    bool in_main;
    bool in_while;
    bool in_if;
    bool return_used;
    DataType func_type;
} Context;

AstBlock *sem_block(Vec stmts, bool top_level);
AstStmt *sem_if(AstCondition *cond, AstBlock *true_block, AstBlock *false_block);
AstCondition *sem_expr_condition(AstExpr *expr);
AstCondition *sem_let_condition(SymItem *ident);
AstExpr *sem_lex_variable(Lexer *lex);
AstExpr *sem_lex_literal(Lexer *lex);
AstStmt *sem_while(AstExpr *cond, AstBlock *loop);
AstStmt *sem_var_decl(bool mutable, SymItem *ident, DataType type, AstExpr *expr);
AstStmt *sem_func_decl(SymItem *ident, Vec params, DataType return_type, AstBlock *body);
bool sem_func_param(String label, SymItem *ident, DataType type, FuncParam *res);
AstStmt *sem_return(AstExpr *expr);

#endif // SEMANTICS_H_INCLUDED
