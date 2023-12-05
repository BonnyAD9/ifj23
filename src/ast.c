#include "ast.h" // Vec, DataType::*

#include "errors.h" // set_err_code, ERR_OTHER
#include "utils.h"  // EPRINTF

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

AstBlock *ast_block(FilePos pos, Vec stmts) {
    STRUCT_ALLOC(AstBlock,
        .stmts = stmts,
        .pos = pos,
    );
}

AstBinaryOp *ast_binary_op(FilePos pos, Token operator, AstExpr *left, AstExpr *right) {
    STRUCT_ALLOC(AstBinaryOp,
        .operator = operator,
        .left = left,
        .right = right,
        .sema_checked = false,
        .pos = pos,
    );
}

AstUnaryOp *ast_unary_op(FilePos pos, Token operator, AstExpr *param) {
    STRUCT_ALLOC(AstUnaryOp,
        .operator = operator,
        .param = param,
        .sema_checked = false,
        .pos = pos,
    );
}

AstFuncCallParam *ast_func_call_var_param(FilePos pos, SymItem *ident, String name) {
    STRUCT_ALLOC(AstFuncCallParam,
        .type = AST_VARIABLE,
        .name = name,
        .variable = ident,
        .pos = pos,
    );
}

AstFuncCallParam *ast_func_call_lit_param(FilePos pos, AstLiteral literal, String name) {
    STRUCT_ALLOC(AstFuncCallParam,
        .type = AST_LITERAL,
        .name = name,
        .literal = literal,
        .pos = pos,
    );
}

AstFunctionCall *ast_function_call(FilePos pos, SymItem *ident, Vec parameters) {
    STRUCT_ALLOC(AstFunctionCall,
        .ident = ident,
        .arguments = parameters,
        .sema_checked = false,
        .pos = pos,
    );
}

AstFunctionDecl *ast_function_decl(FilePos pos, SymItem *ident, Vec parameters, AstBlock *body) {
    STRUCT_ALLOC(AstFunctionDecl,
        .ident = ident,
        .parameters = parameters,
        .body = body,
        .sema_checked = false,
        .pos = pos,
    );
}

AstReturn *ast_return(FilePos pos, AstExpr *expr) {
    STRUCT_ALLOC(AstReturn,
        .expr = expr,
        .sema_checked = false,
        .pos = pos,
    );
}

AstCondition *ast_expr_condition(AstExpr *expr) {
    STRUCT_ALLOC(AstCondition,
        .type = AST_COND_EXPR,
        .expr = expr,
        .sema_checked = false,
        .pos = expr->pos,
    );
}

AstCondition *ast_let_condition(FilePos pos, SymItem *let) {
    STRUCT_ALLOC(AstCondition,
        .type = AST_COND_LET,
        .let = let,
        .sema_checked = false,
        .pos = pos,
    );
}

AstIf *ast_if(FilePos pos, AstCondition *condition, AstBlock *if_body, AstBlock *else_body) {
    STRUCT_ALLOC(AstIf,
        .condition = condition,
        .if_body = if_body,
        .else_body = else_body,
        .sema_checked = false,
        .pos = pos,
    );
}

AstWhile *ast_while(FilePos pos, AstCondition *condition, AstBlock *body) {
    STRUCT_ALLOC(AstWhile,
        .condition = condition,
        .body = body,
        .sema_checked = false,
        .pos = pos,
    );
}

AstLiteral *ast_int_literal(FilePos pos, int value) {
    STRUCT_ALLOC(AstLiteral,
        .data_type = DT_INT,
        .int_v = value,
        .sema_checked = false,
        .pos = pos,
    );
}

AstLiteral *ast_double_literal(FilePos pos, double value) {
    STRUCT_ALLOC(AstLiteral,
        .data_type = DT_DOUBLE,
        .double_v = value,
        .sema_checked = false,
        .pos = pos,
    );
}

AstLiteral *ast_string_literal(FilePos pos, String value) {
    STRUCT_ALLOC(AstLiteral,
        .data_type = DT_STRING,
        .string_v = value,
        .sema_checked = false,
        .pos = pos,
    );
}

AstLiteral *ast_nil_literal(FilePos pos) {
    STRUCT_ALLOC(AstLiteral,
        .data_type = DT_ANY_NIL,
        .sema_checked = false,
        .pos = pos,
    );
}

AstVariable *ast_variable(FilePos pos, SymItem *ident) {
    STRUCT_ALLOC(AstVariable,
        .ident = ident,
        .sema_checked = false,
        .pos = pos,
    );
}

AstVariableDecl *ast_variable_decl(FilePos pos, SymItem *ident, AstExpr *value) {
    STRUCT_ALLOC(AstVariableDecl,
        .ident = ident,
        .value = value,
        .sema_checked = false,
        .pos = pos,
    );
}

AstExpr *ast_binary_op_expr(FilePos pos, AstBinaryOp *value) {
    STRUCT_ALLOC(AstExpr,
        .type = AST_BINARY_OP,
        .binary_op = value,
        .sema_checked = false,
        .pos = pos,
    );
}

AstExpr *ast_unary_op_expr(FilePos pos, AstUnaryOp *value) {
    STRUCT_ALLOC(AstExpr,
        .type = AST_UNARY_OP,
        .unary_op = value,
        .sema_checked = false,
        .pos = pos,
    );
}

AstExpr *ast_function_call_expr(FilePos pos, AstFunctionCall *value) {
    STRUCT_ALLOC(AstExpr,
        .type = AST_FUNCTION_CALL,
        .function_call = value,
        .sema_checked = false,
        .pos = pos,
    );
}

AstExpr *ast_literal_expr(FilePos pos, AstLiteral *value) {
    STRUCT_ALLOC(AstExpr,
        .type = AST_LITERAL,
        .literal = value,
        .sema_checked = false,
        .pos = pos,
    );
}

AstExpr *ast_variable_expr(FilePos pos, AstVariable *value) {
    STRUCT_ALLOC(AstExpr,
        .type = AST_VARIABLE,
        .variable = value,
        .sema_checked = false,
        .pos = pos,
    );
}

AstStmt *ast_expr_stmt(AstExpr *value) {
    STRUCT_ALLOC(AstStmt,
        .type = AST_EXPR,
        .expr = value,
        .sema_checked = false,
        .pos = value->pos,
    );
}

AstStmt *ast_block_stmt(FilePos pos, AstBlock *value) {
    STRUCT_ALLOC(AstStmt,
        .type = AST_BLOCK,
        .block = value,
        .sema_checked = false,
        .pos = pos,
    );
}

AstStmt *ast_function_decl_stmt(FilePos pos, AstFunctionDecl *value) {
    STRUCT_ALLOC(AstStmt,
        .type = AST_FUNCTION_DECL,
        .function_decl = value,
        .sema_checked = false,
        .pos = pos,
    );
}

AstStmt *ast_variable_decl_stmt(FilePos pos, AstVariableDecl *value) {
    STRUCT_ALLOC(AstStmt,
        .type = AST_VARIABLE_DECL,
        .variable_decl = value,
        .sema_checked = false,
        .pos = pos,
    );
}

AstStmt *ast_return_stmt(FilePos pos, AstReturn *value) {
    STRUCT_ALLOC(AstStmt,
        .type = AST_RETURN,
        .return_v = value,
        .sema_checked = false,
        .pos = pos,
    );
}

AstStmt *ast_if_stmt(FilePos pos, AstIf *value) {
    STRUCT_ALLOC(AstStmt,
        .type = AST_IF,
        .if_v = value,
        .sema_checked = false,
        .pos = pos,
    );
}

AstStmt *ast_while_stmt(FilePos pos, AstWhile *value) {
    STRUCT_ALLOC(AstStmt,
        .type = AST_WHILE,
        .while_v = value,
        .sema_checked = false,
        .pos = pos,
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

void ast_free_func_call_param(AstFuncCallParam *value) {
    str_free(&value->name);
    if (value->type == AST_LITERAL && value->literal.data_type == DT_STRING) {
        str_free(&value->literal.string_v);
    }
}

void ast_free_function_call(AstFunctionCall **value) {
    FREE_INIT(AstFunctionCall, value, v);

    vec_free_with(&v->arguments, (FreeFun)ast_free_func_call_param);
    free(v);
}

void ast_free_function_decl(AstFunctionDecl **value) {
    FREE_INIT(AstFunctionDecl, value, v);

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

    if (v->type == AST_COND_EXPR)
        ast_free_expr(&v->expr);

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

    if (v->data_type == DT_STRING) {
        str_free(&v->string_v);
    }
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
