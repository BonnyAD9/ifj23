/**
 * IFJ23
 *
 * xdanie14 Tomáš Daniel
 * xsleza23 Martin Slezák
 * xstigl00 Jakub Antonín Štigler
 * xvrbas01 Milan Vrbas
 */

#ifndef AST_H_INCLUDED
#define AST_H_INCLUDED

#include "vec.h"      // Vec
#include "symtable.h" // SymItem
#include "enums.h"    // DataType
#include "stream.h"   // FilePos
// Forward declarations

// Expression, something that has value
typedef struct AstExpr AstExpr;
// Statement something that doesn't have value
typedef struct AstStmt AstStmt;

typedef struct {
    // type: AstStmt *
    Vec stmts;

    // position of the start of the block
    FilePos pos;
} AstBlock;

typedef struct {
    Token operator;
    AstExpr *left;
    AstExpr *right;

    DataType data_type; // Undefined value unless sema_checked is true!
    bool sema_checked;

    // position of the operator character
    FilePos pos;
} AstBinaryOp;

typedef struct {
    Token operator;
    AstExpr *param;

    DataType data_type; // Undefined value unless sema_checked is true!
    bool sema_checked;

    // position of the operator character
    FilePos pos;
} AstUnaryOp;

typedef struct {
    DataType data_type; // Already defined from constructor
    union {
        int int_v;
        double double_v;
        String string_v;
    };

    bool sema_checked;

    FilePos pos;
} AstLiteral;

typedef enum {
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_FUNCTION_CALL,
    AST_LITERAL,
    AST_VARIABLE,
} AstExprType;

typedef struct {
    String name;
    // can be only literal or variable
    AstExprType type;
    union {
        SymItem *variable;
        AstLiteral literal;
    };

    FilePos pos;
} AstFuncCallParam;

typedef struct {
    SymItem *ident;
    // type: AstFuncCallParam
    Vec arguments;

    // func type from table
    DataType data_type; // Undefined value unless sema_checked is true!
    bool sema_checked;

    // position of the open bracket in the call
    FilePos pos;
} AstFunctionCall;

typedef struct {
    SymItem *ident;
    Vec parameters;
    AstBlock *body;
    // func type from table
    bool sema_checked;

    // position of the func keyword
    FilePos pos;
} AstFunctionDecl;

typedef struct {
    AstExpr *expr;

    DataType data_type; // Undefined value unless sema_checked is true!
    bool sema_checked;

    // position of the return keyword
    FilePos pos;
} AstReturn;

typedef enum {
    AST_COND_EXPR,
    AST_COND_LET,
} AstConditionType;

typedef struct {
    SymItem *ident;
    AstExpr *value;

    bool sema_checked;

    // position of the let/var keyword
    FilePos pos;
} AstVariableDecl;

typedef struct {
    AstConditionType type;
    union {
        AstExpr *expr;
        SymItem *let;
    };

    bool sema_checked;

    // position of the first token of the condition
    FilePos pos;
} AstCondition;

typedef struct {
    AstCondition *condition;
    AstBlock *if_body;
    AstBlock *else_body;

    bool sema_checked;

    // position of the if keyword
    FilePos pos;
} AstIf;

typedef struct {
    AstCondition *condition;
    AstBlock *body;

    bool sema_checked;

    // position of the while keyword
    FilePos pos;
} AstWhile;

typedef struct {
    SymItem *ident;

    // data type from table
    DataType data_type; // Undefined value unless sema_checked is true!
    bool sema_checked;

    // position of the identifier
    FilePos pos;
} AstVariable;

struct AstExpr {
    AstExprType type;
    union {
        AstBinaryOp *binary_op;
        AstUnaryOp *unary_op;
        AstFunctionCall *function_call;
        AstLiteral *literal;
        AstVariable *variable;
    };

    DataType data_type; // Undefined value unless sema_checked is true!
    bool sema_checked;

    // position of the first token of the expression
    FilePos pos;
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

    DataType data_type; // Undefined value unless sema_checked is true!
    bool sema_checked;

    // position of the first token of the statement
    FilePos pos;
};

AstBlock *ast_block(FilePos pos, Vec stmts);

AstBinaryOp *ast_binary_op(FilePos pos, Token operator, AstExpr *left, AstExpr *right);

AstUnaryOp *ast_unary_op(FilePos pos, Token operator, AstExpr *param);

AstFuncCallParam *ast_func_call_var_param(FilePos pos, SymItem *ident, String name);

AstFuncCallParam *ast_func_call_lit_param(FilePos pos, AstLiteral literal, String name);

AstFunctionCall *ast_function_call(FilePos pos, SymItem *ident, Vec parameters);

AstFunctionDecl *ast_function_decl(FilePos pos, SymItem *ident, Vec parameters, AstBlock *body);

AstReturn *ast_return(FilePos pos, AstExpr *expr);

AstCondition *ast_expr_condition(AstExpr *expr);

AstCondition *ast_let_condition(FilePos pos, SymItem *let);

AstIf *ast_if(FilePos pos, AstCondition *condition, AstBlock *if_body, AstBlock *else_body);

AstWhile *ast_while(FilePos pos, AstCondition *condition, AstBlock *body);

AstLiteral *ast_int_literal(FilePos pos, int value);

AstLiteral *ast_double_literal(FilePos pos, double value);

AstLiteral *ast_string_literal(FilePos pos, String value);

AstLiteral *ast_nil_literal(FilePos pos);

AstVariable *ast_variable(FilePos pos, SymItem *ident);

AstVariableDecl *ast_variable_decl(FilePos pos, SymItem *ident, AstExpr *value);

AstExpr *ast_binary_op_expr(FilePos pos, AstBinaryOp *value);

AstExpr *ast_unary_op_expr(FilePos pos, AstUnaryOp *value);

AstExpr *ast_function_call_expr(FilePos pos, AstFunctionCall *value);

AstExpr *ast_literal_expr(FilePos pos, AstLiteral *value);

AstExpr *ast_variable_expr(FilePos pos, AstVariable *value);

AstStmt *ast_expr_stmt(AstExpr *value);

AstStmt *ast_block_stmt(FilePos pos, AstBlock *value);

AstStmt *ast_function_decl_stmt(FilePos pos, AstFunctionDecl *value);

AstStmt *ast_variable_decl_stmt(FilePos pos, AstVariableDecl *value);

AstStmt *ast_return_stmt(FilePos pos, AstReturn *value);

AstStmt *ast_if_stmt(FilePos pos, AstIf *value);

AstStmt *ast_while_stmt(FilePos pos, AstWhile *value);

void ast_free_block(AstBlock **value);

void ast_free_binary_op(AstBinaryOp **value);

void ast_free_unary_op(AstUnaryOp **value);

void ast_free_func_call_param(AstFuncCallParam *value);

void ast_free_function_call(AstFunctionCall **value);

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
