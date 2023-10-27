#ifndef AST
#define AST

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

typedef enum {
    INTEGER_TYPE_EXPR,      
    DOUBLE_TYPE_EXPR,       
    STRING_TYPE_EXPR,       
    INTEGER_EXPR,           
    DOUBLE_EXPR,            
    STRING_EXPR,            
    VARIABLE_EXPR,          
    BINARY_OP_EXPR,         
    ASSIGN_EXPR,            
    FUNCION_CALL_EXPR,      
    FUNCION_DEF_EXPR,       
    RETURN_EXPR,            
    CONDITIONS_EXPR,        
    WHILE_EXPR,             
    VARIABLE_DEF_EXPR,      
    VARIABLE_DEC_EXPR,      
    FUNCTION_PARAM_EXPR,    
    FUNCTION_ARG_EXPR,      
    BODY_EXPR,
} NodeExprType;

typedef struct Expressions {
    NodeExprType expression_type;
    union {
        struct Expressions* integer_type;   
        struct Expressions* double_type;    
        struct Expressions* string_type;    
        int integer_expr;                   
        double double_expr;                 
        char* string_expr;                  
        char* variable_expr;                
        struct {
            char* operator_expr;
            struct Expressions* left;
            struct Expressions* right;
        } OperatorExpr;                     
        struct {
            char* assign_expr;
            struct Expressions* left;
            struct Expressions* right;
        } AssignExpr;                       
        struct {
            char* func_call_name_expr;
            struct Expression_list* func_arguments;
        } FunctionCallExpr;                  
        struct {
            char* func_def_name_expr;
            struct Expression_list* func_parameters;
            struct Expressions* func_body;
        } FunctionDefExpr;                   
        struct Expressions* return_expr;     
        struct {
            struct Expressions* if_condition;
            struct Expressions* if_body;
            struct Expressions* else_body;
        } ConditionsExpr;                    
        struct {
            struct Expressions* while_condition;
            struct Expressions* while_body;
        } WhileExpr;                         
        struct {
            char* var_def_name_expr;
            struct Expressions* varible_type;
            struct Expressions* variable_value;
        } VariableDefExpr;                   
        struct {
            char* var_dec_name_expr;
            struct Expressions* varible_type;
            struct Expressions* variable_value;
        } VariableDecExpr;                   
        
        struct {
            struct Expressions* arg_value;
            //struct ArgumentExpr* next;
        } ArgumentExpr;                       
        struct {
            struct Expressions* param_name;
            struct Expressions* param_type;
            //struct ParameterExpr* next;
        } ParameterExpr;                       
        struct {
            struct Expressions* body; 
            struct Expression_list* next;
        } BodyExpr;
    } ExpressionUnion;
} AbstractSyntaxTree;

    typedef struct Expression_list {
        AbstractSyntaxTree node;
        struct Expression_list* next;
    } ASTList;

void free_ast(AbstractSyntaxTree* node);

void free_ast_list(ASTList* list);

void preorder_traversal(AbstractSyntaxTree* node);

AbstractSyntaxTree* node_fail(AbstractSyntaxTree* node);

AbstractSyntaxTree* make_binaryExpr(char* operator_expr, AbstractSyntaxTree* left, AbstractSyntaxTree* right);

AbstractSyntaxTree* make_variableExpr(char* variable_name);

AbstractSyntaxTree* make_integerExpr(int value);

AbstractSyntaxTree* make_integerType(AbstractSyntaxTree* value);

AbstractSyntaxTree* make_doubleExpr(double value);

AbstractSyntaxTree* make_doubleType(AbstractSyntaxTree* value);

AbstractSyntaxTree* make_stringExpr(char* value);

AbstractSyntaxTree* make_stringType(AbstractSyntaxTree* value);

AbstractSyntaxTree* make_returnExpr(AbstractSyntaxTree* return_value);

AbstractSyntaxTree* make_whileExpr(AbstractSyntaxTree* while_condition, AbstractSyntaxTree* while_body);

AbstractSyntaxTree* make_conditionExpr(AbstractSyntaxTree* if_condition, AbstractSyntaxTree* if_body, AbstractSyntaxTree* else_body);

AbstractSyntaxTree* make_function_callExpr(char* func_call_name_expr, ASTList* func_arguments);

AbstractSyntaxTree* make_function_defExpr(char* func_def_name_expr, ASTList* func_parameters, AbstractSyntaxTree* func_body);

AbstractSyntaxTree* make_variable_defExpr(char* var_def_name_expr, AbstractSyntaxTree* varible_type, AbstractSyntaxTree* variable_value);

AbstractSyntaxTree* make_variable_decExpr(char* var_dec_name_expr, AbstractSyntaxTree* varible_type, AbstractSyntaxTree* variable_value);

AbstractSyntaxTree* make_ArgumentExpr(AbstractSyntaxTree* arg_name);

AbstractSyntaxTree* make_ParameterExpr(AbstractSyntaxTree* param_name, AbstractSyntaxTree* param_type);

AbstractSyntaxTree* make_BodyExpr(AbstractSyntaxTree* body, ASTList* next);

#endif