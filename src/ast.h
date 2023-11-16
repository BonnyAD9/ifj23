#ifndef AST_H_INCLUDED
#define AST_H_INCLUDED

#include "vec.h"
#include "lexer.h"

// TODO: symbol table type
typedef int SymItem;

// Forward declarations

// Expression, something that has value
typedef struct AstExpr AstExpr;
// Statement something that doesn't have value
typedef struct AstStmt AstStmt;

typedef struct {
    // type: AstStmt *
    Vec stmts;
} AstBlock;

typedef struct {
    Token operator;
    AstExpr *left;
    AstExpr *right;
} AstBinaryOp;

typedef struct {
    Token operator;
    AstExpr *param;
} AstUnaryOp;

typedef enum {
    ASTFCP_VARIABLE,
    ASTFCP_LITERAL,
} AstFuncCallParamType;

typedef struct {
    SymItem *name;
    AstFuncCallParamType type;
    union {
        SymItem *variable;
        AstLiteral *literal;
    };
} AstFuncCallParam;

typedef struct {
    SymItem *ident;
    // type: AstFuncCallParam *
    Vec arguments;
} AstFunctionCall;

typedef enum {
    LITERAL_INT,
    LITERAL_DOUBLE,
    LITERAL_STRING,
    LITERAL_NIL,
} AstType;

typedef struct {
    SymItem *ident;
    SymItem *name;
} AstFuncDeclParam;

typedef struct {
    SymItem *ident;
    // type: AstFuncDeclParam
    Vec parameters;
    AstBlock *body;
} AstFunctionDecl;

typedef struct {
    AstExpr *expr;
} AstReturn;

typedef enum {
    AST_COND_EXPR,
    AST_COND_LET,
} AstConditionType;

typedef struct {
    SymItem *ident;
    AstExpr *value;
} AstVariableDecl;

typedef struct {
    AstConditionType type;
    union {
        AstExpr *expr;
        AstVariableDecl *let;
    };
} AstCondition;

typedef struct {
    AstCondition *condition;
    AstBlock *if_body;
    AstBlock *else_body;
} AstIf;

typedef struct {
    AstCondition *condition;
    AstBlock *body;
} AstWhile;

typedef struct {
    AstType type;
    union {
        int int_v;
        double double_v;
        String string_v;
    };
} AstLiteral;

typedef struct {
    SymItem *ident;
} AstVariable;

typedef enum {
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_FUNCTION_CALL,
    AST_LITERAL,
    AST_VARIABLE,
} AstExprType;

struct AstExpr {
    AstExprType type;
    union {
        AstBinaryOp *binary_op;
        AstUnaryOp *unary_op;
        AstFunctionCall *function_call;
        AstLiteral *literal;
        AstVariable *variable;
    };
};

typedef enum {
    AST_EXPR,
    AST_BLOCK,
    AST_FUNCTION_DECL,
    AST_VARIABLE_DECL,
    AST_RETURN,
    AST_IF,
    AST_WHILE,
} AstStmtType;

struct AstStmt {
    AstStmtType type;
    union {
        AstExpr *expr;
        AstBlock *block;
        AstFunctionDecl *function_decl;
        AstVariableDecl *variable_decl;
        AstReturn *return_v;
        AstIf *if_v;
        AstWhile *while_v;
    };
};

AstBlock *ast_block(Vec stmts);

AstBinaryOp *ast_binary_op(Token operator, AstExpr *left, AstExpr *right);

AstUnaryOp *ast_unary_op(Token operator, AstExpr *param);

AstFuncCallParam *ast_func_call_var_param(SymItem *ident, SymItem *name);

AstFuncCallParam *ast_func_call_lit_param(AstLiteral *literal, SymItem *name);

AstFunctionCall *ast_function_call(SymItem *ident, Vec parameters);

AstFuncDeclParam *ast_func_decl_param(SymItem *ident, SymItem *name);

AstFunctionDecl *ast_function_decl(SymItem *ident, AstBlock *body);

AstReturn *ast_return(AstExpr *expr);

AstCondition *ast_expr_condition(AstExpr *expr);

AstCondition *ast_let_condition(AstVariableDecl *let);

AstIf *ast_if(AstCondition *condition, AstBlock *if_body, AstBlock *else_body);

AstWhile *ast_while(AstCondition *condition, AstBlock *body);

AstLiteral *ast_int_literal(int value);

AstLiteral *ast_double_literal(double value);

AstLiteral *ast_string_literal(String value);

AstLiteral *ast_nil_literal();

AstVariable *ast_variable(SymItem *ident);

AstVariableDecl *ast_variable_decl(SymItem *ident, AstExpr *value);

AstExpr *ast_binary_op_expr(AstBinaryOp *value);

AstExpr *ast_unary_op_expr(AstUnaryOp *value);

AstExpr *ast_function_call_expr(AstFunctionCall *value);

AstExpr *ast_literal_expr(AstLiteral *value);

AstExpr *ast_variable_expr(AstVariable *value);

AstStmt *ast_expr_stmt(AstExpr *value);

AstStmt *ast_block_stmt(AstBlock *value);

AstStmt *ast_function_decl_stmt(AstFunctionDecl *value);

AstStmt *ast_variable_decl_stmt(AstVariableDecl *value);

AstStmt *ast_return_stmt(AstReturn *value);

AstStmt *ast_if_stmt(AstIf *value);

AstStmt *ast_while_stmt(AstWhile *value);

void ast_free_block(AstBlock **value);

void ast_free_binary_op(AstBinaryOp **value);

void ast_free_unary_op(AstUnaryOp **value);

void ast_free_func_call_param(AstFuncCallParam **value);

void ast_free_function_call(AstFunctionCall **value);

void ast_free_func_decl_param(AstFuncDeclParam **value);

void ast_free_function_decl(AstFunctionDecl **value);

void ast_free_function_decl(AstFunctionDecl **value);

void ast_free_return(AstReturn **value);

void ast_free_condition(AstCondition **value);

void ast_free_if(AstIf **value);

void ast_free_while(AstWhile **value);

void ast_free_literal(AstLiteral **value);

void ast_free_variable(AstVariable **value);

void ast_free_variable_decl(AstVariableDecl **value);

void ast_free_expr(AstExpr **value);

void ast_free_stmt(AstStmt **value);

#endif // AST_H_INCLUDED
