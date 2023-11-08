#include "ast.h"

#include "errors.h"
#include "utils.h"

#include <stdlib.h>

static void *ast_err(const char *msg) {
    set_err_code(ERR_OTHER);
    EPRINTF("%s", msg);
    return NULL;
}

#define STRUCT_ALLOC(type, ...) \
    type *res = malloc(sizeof(*res)); \
    if (!res) { \
        return ast_err("Failed to allocate memory"); \
    } \
    *res = (type) { \
        __VA_ARGS__ \
    }; \
    return res

AstBlock *ast_block(Vec stmts) {
    STRUCT_ALLOC(AstBlock,
        .stmts = stmts,
    );
}

AstBinaryOp *ast_binary_op(Token operator, AstExpr *left, AstExpr *right) {
    STRUCT_ALLOC(AstBinaryOp,
        .operator = operator,
        .left = left,
        .right = right,
    );
}

AstUnaryOp *ast_unary_op(Token operator, AstExpr *param) {
    STRUCT_ALLOC(AstUnaryOp,
        .operator = operator,
        .param = param,
    );
}

AstFuncCallParam *ast_func_call_param(SymItem *ident, SymItem *name) {
    STRUCT_ALLOC(AstFuncCallParam,
        .ident = ident,
        .name = name,
    );
}

AstFunctionCall *ast_function_call(SymItem *ident, Vec parameters) {
    STRUCT_ALLOC(AstFunctionCall,
        .ident = ident,
        .arguments = parameters,
    );
}

AstFuncDeclParam *ast_func_decl_param(SymItem *ident, SymItem *name) {
    STRUCT_ALLOC(AstFuncDeclParam,
        .ident = ident,
        .name = name,
    );
}

AstFunctionDecl *ast_function_decl(SymItem *ident, AstBlock *body) {
    STRUCT_ALLOC(AstFunctionDecl,
        .ident = ident,
        .body = body,
    );
}

AstReturn *ast_return(AstExpr *expr) {
    STRUCT_ALLOC(AstReturn,
        .expr = expr,
    );
}

AstCondition *ast_expr_condition(AstExpr *expr) {
    STRUCT_ALLOC(AstCondition,
        .type = AST_COND_EXPR,
        .expr = expr,
    );
}

AstCondition *ast_let_condition(AstVariableDecl *let) {
    STRUCT_ALLOC(AstCondition,
        .type = AST_COND_LET,
        .let = let,
    );
}

AstIf *ast_if(AstCondition *condition, AstBlock *if_body, AstBlock *else_body) {
    STRUCT_ALLOC(AstIf,
        .condition = condition,
        .if_body = if_body,
        .else_body = else_body,
    );
}

AstWhile *ast_while(AstCondition *condition, AstBlock *body) {
    STRUCT_ALLOC(AstWhile,
        .condition = condition,
        .body = body,
    );
}

AstLiteral *ast_int_literal(int value) {
    STRUCT_ALLOC(AstLiteral,
        .type = LITERAL_INT,
        .int_v = value,
    );
}

AstLiteral *ast_double_literal(double value) {
    STRUCT_ALLOC(AstLiteral,
        .type = LITERAL_DOUBLE,
        .double_v = value,
    );
}

AstLiteral *ast_string_literal(String value) {
    STRUCT_ALLOC(AstLiteral,
        .type = LITERAL_STRING,
        .string_v = value,
    );
}

AstLiteral *ast_nil_literal() {
    STRUCT_ALLOC(AstLiteral,
        .type = LITERAL_NIL,
    );
}

AstVariable *ast_variable(SymItem *ident) {
    STRUCT_ALLOC(AstVariable,
        .ident = ident,
    );
}

AstVariableDecl *ast_variable_decl(SymItem *ident, AstExpr *value) {
    STRUCT_ALLOC(AstVariableDecl,
        .ident = ident,
        .value = value,
    );
}

AstExpr *ast_binary_op_expr(AstBinaryOp *value) {
    STRUCT_ALLOC(AstExpr,
        .type = AST_BINARY_OP,
        .binary_op = value,
    );
}

AstExpr *ast_unary_op_expr(AstUnaryOp *value) {
    STRUCT_ALLOC(AstExpr,
        .type = AST_UNARY_OP,
        .unary_op = value,
    );
}

AstExpr *ast_function_call_expr(AstFunctionCall *value) {
    STRUCT_ALLOC(AstExpr,
        .type = AST_FUNCTION_CALL,
        .function_call = value,
    );
}

AstExpr *ast_literal_expr(AstLiteral *value) {
    STRUCT_ALLOC(AstExpr,
        .type = AST_LITERAL,
        .literal = value,
    );
}

AstExpr *ast_variable_expr(AstVariable *value) {
    STRUCT_ALLOC(AstExpr,
        .type = AST_VARIABLE,
        .variable = value,
    );
}

AstStmt *ast_expr_stmt(AstExpr *value) {
    STRUCT_ALLOC(AstStmt,
        .type = AST_EXPR,
        .expr = value,
    );
}

AstStmt *ast_block_stmt(AstBlock *value) {
    STRUCT_ALLOC(AstStmt,
        .type = AST_BLOCK,
        .block = value,
    );
}

AstStmt *ast_function_decl_stmt(AstFunctionDecl *value) {
    STRUCT_ALLOC(AstStmt,
        .type = AST_FUNCTION_DECL,
        .function_decl = value,
    );
}

AstStmt *ast_variable_decl_stmt(AstVariableDecl *value) {
    STRUCT_ALLOC(AstStmt,
        .type = AST_VARIABLE_DECL,
        .variable_decl = value,
    );
}

AstStmt *ast_return_stmt(AstReturn *value) {
    STRUCT_ALLOC(AstStmt,
        .type = AST_RETURN,
        .return_v = value,
    );
}

AstStmt *ast_if_stmt(AstIf *value) {
    STRUCT_ALLOC(AstStmt,
        .type = AST_IF,
        .if_v = value,
    );
}

AstStmt *ast_while_stmt(AstWhile *value) {
    STRUCT_ALLOC(AstStmt,
        .type = AST_WHILE,
        .while_v = value,
    );
}

#define FREE_INIT(type, value, v) \
    type *v = *value; \
    *value = NULL; \
    if (!v) return \

void ast_free_block(AstBlock **value) {
    FREE_INIT(AstBlock, value, v);

    vec_free_with(&v->stmts, (FreeFun)ast_free_stmt);
    free(v);
}

void ast_free_binary_op(AstBinaryOp **value) {
    FREE_INIT(AstBinaryOp, value, v);

    ast_free_expr(&v->left);
    ast_free_expr(&v->right);
    free(v);
}

void ast_free_unary_op(AstUnaryOp **value) {
    FREE_INIT(AstUnaryOp, value, v);

    ast_free_expr(&v->param);
    free(v);
}

void ast_free_func_call_param(AstFuncCallParam **value) {
    FREE_INIT(AstFuncCallParam, value, v);

    free(v);
}

void ast_free_function_call(AstFunctionCall **value) {
    FREE_INIT(AstFunctionCall, value, v);

    vec_free_with(&v->arguments, (FreeFun)ast_free_func_call_param);
    free(v);
}

void ast_free_func_decl_param(AstFuncDeclParam **value) {
    FREE_INIT(AstFuncDeclParam, value, v);

    free(v);
}

void ast_free_function_decl(AstFunctionDecl **value) {
    FREE_INIT(AstFunctionDecl, value, v);

    vec_free_with(&v->parameters, (FreeFun)ast_free_func_decl_param);
    ast_free_block(&v->body);
    free(v);
}

void ast_free_return(AstReturn **value) {
    FREE_INIT(AstReturn, value, v);

    ast_free_expr(&v->expr);
    free(v);
}

void ast_free_condition(AstCondition **value) {
    FREE_INIT(AstCondition, value, v);

    switch (v->type) {
    case AST_COND_EXPR:
        ast_free_expr(&v->expr);
        break;
    case AST_COND_LET:
        ast_free_variable_decl(&v->let);
        break;
    }
    free(v);
}

void ast_free_if(AstIf **value) {
    FREE_INIT(AstIf, value, v);

    ast_free_condition(&v->condition);
    ast_free_block(&v->if_body);
    ast_free_block(&v->else_body);
    free(v);
}

void ast_free_while(AstWhile **value) {
    FREE_INIT(AstWhile, value, v);

    ast_free_condition(&v->condition);
    ast_free_block(&v->body);
    free(v);
}

void ast_free_literal(AstLiteral **value) {
    FREE_INIT(AstLiteral, value, v);

    free(v);
}

void ast_free_variable(AstVariable **value) {
    FREE_INIT(AstVariable, value, v);

    free(v);
}

void ast_free_variable_decl(AstVariableDecl **value) {
    FREE_INIT(AstVariableDecl, value, v);

    ast_free_expr(&v->value);
    free(v);
}

void ast_free_expr(AstExpr **value) {
    FREE_INIT(AstExpr, value, v);

    switch (v->type) {
    case AST_BINARY_OP:
        ast_free_binary_op(&v->binary_op);
        break;
    case AST_UNARY_OP:
        ast_free_unary_op(&v->unary_op);
        break;
    case AST_FUNCTION_CALL:
        ast_free_function_call(&v->function_call);
        break;
    case AST_LITERAL:
        ast_free_literal(&v->literal);
        break;
    case AST_VARIABLE:
        ast_free_variable(&v->variable);
        break;
    }

    free(v);
}

void ast_free_stmt(AstStmt **value) {
    FREE_INIT(AstStmt, value, v);

    switch (v->type) {
    case AST_EXPR:
        ast_free_expr(&v->expr);
        break;
    case AST_BLOCK:
        ast_free_block(&v->block);
        break;
    case AST_FUNCTION_DECL:
        ast_free_function_decl(&v->function_decl);
        break;
    case AST_VARIABLE_DECL:
        ast_free_variable_decl(&v->variable_decl);
        break;
    case AST_RETURN:
        ast_free_return(&v->return_v);
        break;
    case AST_IF:
        ast_free_if(&v->if_v);
        break;
    case AST_WHILE:
        ast_free_while(&v->while_v);
        break;
    }

    free(v);
}
