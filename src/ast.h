#ifndef AST_H_INCLUDED
#define AST_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "vec.h"
#include "lexer.h"

enum NodeExprType{
    BINARY_OP_EXPR,
    FUNCTION_CALL_EXPR,
    FUNCTION_DEF_EXPR,
    RETURN_EXPR,
    CONDITIONS_EXPR,
    WHILE_EXPR,
    VARIABLE_DEC_EXPR,
    FUNCTION_PARAM_EXPR,
    VARIABLE_VALUE,
};

// Forward declarations
typedef struct AbstractSyntaxTree AbstractSyntaxTree;
typedef struct VariableValue VariableValue;

typedef struct {
    String operator_expr;
    AbstractSyntaxTree* left;
    AbstractSyntaxTree* right;
} OperatorExpr;

typedef struct {
    String func_name;
    Vec* func_arguments;
} FunctionCallExpr;

enum ValueType {
    INT_VALUE,
    DOUBLE_VALUE,
    STRING_VALUE,
    UNKNOWN // Value will be determined from symtable
};

typedef struct {
    String func_name;
    Vec* func_parameters;
    Vec* func_body;
    enum ValueType ret_val;
} FunctionDefExpr;

typedef struct {
    AbstractSyntaxTree* return_expr;
} ReturnExpr;

typedef struct {
    AbstractSyntaxTree* if_condition;
    Vec* if_body;
    Vec* else_body;
} ConditionsExpr;

typedef struct {
    AbstractSyntaxTree* while_condition;
    Vec* while_body;
} WhileExpr;

typedef struct VariableValue {
    enum ValueType type;
    union {
        int int_value;
        double double_value;
        String string_value;
    } ValueUnion;
} VariableValue;

typedef struct {
    String name;
    union {
        VariableValue value;
        AbstractSyntaxTree* right_part;
    } VarUnion;
} VariableExpr;

typedef struct {
    String param_name;
    String param_ident;
    enum ValueType param_type;
} ParameterExpr;

typedef struct AbstractSyntaxTree {
    enum NodeExprType expression_type;
    union {
        OperatorExpr        operator_expr;
        FunctionCallExpr    func_call_expr;
        FunctionDefExpr     func_def_expr;
        ReturnExpr          return_expr;
        ConditionsExpr      conditions_expr;
        WhileExpr           while_expr;
        VariableExpr        var_expr;
        ParameterExpr       parameter_expr;
        VariableValue       variable_value;
    } ExpressionUnion;
} AbstractSyntaxTree;

void free_ast(AbstractSyntaxTree* node);

void free_ast_vec(Vec* vec);

void preorder_traversal(AbstractSyntaxTree* node);

AbstractSyntaxTree* make_binaryExpr(String operator_expr, AbstractSyntaxTree* left, AbstractSyntaxTree* right);

AbstractSyntaxTree* make_returnExpr(AbstractSyntaxTree* return_value);

AbstractSyntaxTree* make_whileExpr(AbstractSyntaxTree* while_condition, Vec* while_body);

AbstractSyntaxTree* make_conditionExpr(AbstractSyntaxTree* if_condition, Vec* if_body, Vec* else_body);

AbstractSyntaxTree* make_function_callExpr(String func_name, Vec* func_arguments);

AbstractSyntaxTree* make_function_defExpr(String func_name, Vec* func_parameters, Vec* func_body, enum ValueType ret_val);

AbstractSyntaxTree* make_variable_Expr(String var_name, enum ValueType type, AbstractSyntaxTree* right_part);

AbstractSyntaxTree* make_valueExpr(String arg_name, enum ValueType type, Lexer* lex);

AbstractSyntaxTree* make_parameterExpr(String param_name, String param_ident, enum ValueType param_type);

#endif // AST_H_INCLUDED