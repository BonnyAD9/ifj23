#include "ast.h"

void free_ast_vec(Vec* vec) {
    if (!vec)
        return;
    VEC_FOR_EACH(vec, AbstractSyntaxTree*, item)
        free_ast(*(item.v));
    vec_free(vec);
}

void free_ast(AbstractSyntaxTree* node) {
    if (!node)
        return;
    // String type variables will be delivered to tree by Lexer and also freed, not here
    switch (node->expression_type) {
        case BINARY_OP_EXPR:
            free_ast(node->ExpressionUnion.operator_expr.left);
            free_ast(node->ExpressionUnion.operator_expr.right);
            break;
        case FUNCTION_CALL_EXPR:
            free_ast_vec(node->ExpressionUnion.func_call_expr.func_arguments);
            break;
        case FUNCTION_DEF_EXPR:
            free_ast_vec(node->ExpressionUnion.func_def_expr.func_parameters);
            free_ast_vec(node->ExpressionUnion.func_def_expr.func_body);
            break;
        case RETURN_EXPR:
            free_ast(node->ExpressionUnion.return_expr.return_expr);
            break;
        case CONDITIONS_EXPR:
            free_ast(node->ExpressionUnion.conditions_expr.if_condition);
            free_ast_vec(node->ExpressionUnion.conditions_expr.if_body);
            free_ast_vec(node->ExpressionUnion.conditions_expr.else_body);
            break;
        case WHILE_EXPR:
            free_ast(node->ExpressionUnion.while_expr.while_condition);
            free_ast_vec(node->ExpressionUnion.while_expr.while_body);
            break;
        case VARIABLE_DEC_EXPR:
            free_ast(node->ExpressionUnion.var_expr.VarUnion.right_part);
            break;
        default:
            break;
    }
    free(node);
}

void preorder_traversal(AbstractSyntaxTree* node) {
    if (!node)
        return;

    switch (node->expression_type) {
        case BINARY_OP_EXPR:
            printf("Binary Operation: %s\n", node->ExpressionUnion.operator_expr.operator_expr.str);
            preorder_traversal(node->ExpressionUnion.operator_expr.left);
            preorder_traversal(node->ExpressionUnion.operator_expr.right);
            break;
        case FUNCTION_CALL_EXPR:
            printf("Function call - function name: %s\n", node->ExpressionUnion.func_call_expr.func_name.str);
            printf("Function arguments: \n");
            VEC_FOR_EACH(node->ExpressionUnion.func_call_expr.func_arguments, AbstractSyntaxTree*, item)
                preorder_traversal(*(item.v));
            break;
        case FUNCTION_DEF_EXPR:
            printf("Function definition - function name: %s\n", node->ExpressionUnion.func_def_expr.func_name.str);
            printf("Function params: \n");
            VEC_FOR_EACH(node->ExpressionUnion.func_def_expr.func_parameters, AbstractSyntaxTree*, item)
                preorder_traversal(*(item.v));
            printf("Function body: \n");
            VEC_FOR_EACH(node->ExpressionUnion.func_def_expr.func_body, AbstractSyntaxTree*, item)
                preorder_traversal(*(item.v));
            break;
        case RETURN_EXPR:
            printf("Return expression\n");
            preorder_traversal(node->ExpressionUnion.return_expr.return_expr);
            break;
        case CONDITIONS_EXPR:
            printf("Condition expression\n");
            printf("If condition: \n");
            preorder_traversal(node->ExpressionUnion.conditions_expr.if_condition);
            printf("If body: \n");
            VEC_FOR_EACH(node->ExpressionUnion.conditions_expr.if_body, AbstractSyntaxTree*, item)
                preorder_traversal(*(item.v));
            printf("Else body: \n");
            VEC_FOR_EACH(node->ExpressionUnion.conditions_expr.else_body, AbstractSyntaxTree*, item)
                preorder_traversal(*(item.v));
            break;
        case WHILE_EXPR:
            printf("While expression\n");
            printf("While condition\n");
            preorder_traversal(node->ExpressionUnion.while_expr.while_condition);
            printf("While body\n");
            VEC_FOR_EACH(node->ExpressionUnion.while_expr.while_body, AbstractSyntaxTree*, item)
                preorder_traversal(*(item.v));
            break;
        case VARIABLE_DEC_EXPR:
            printf("Variable expression - name: %s\n", node->ExpressionUnion.var_expr.name.str);
            preorder_traversal(node->ExpressionUnion.var_expr.VarUnion.right_part);
            break;
        case FUNCTION_PARAM_EXPR:
            printf("Param expression - name: %s\n", node->ExpressionUnion.parameter_expr.param_name.str);
            printf("Param expression - ident: %s\n", node->ExpressionUnion.parameter_expr.param_ident.str);
            break;
        case VARIABLE_VALUE:
            if (node->ExpressionUnion.variable_value.type == INT_VALUE)
                printf("Integer Expression: %d\n", node->ExpressionUnion.var_expr.VarUnion.value.ValueUnion.int_value);
            else if (node->ExpressionUnion.variable_value.type == DOUBLE_VALUE)
                printf("Double Expression: %f\n", node->ExpressionUnion.var_expr.VarUnion.value.ValueUnion.double_value);
            else
                printf("String Expression: %s\n", node->ExpressionUnion.var_expr.VarUnion.value.ValueUnion.string_value.str);
            break;
        default:
            break;
    }
}

/*************************************************************************************************************/
// "left operator_expr[<, >, ...] right"

AbstractSyntaxTree* make_binaryExpr(String operator_expr, AbstractSyntaxTree* left, AbstractSyntaxTree* right) {
    AbstractSyntaxTree* node = malloc(sizeof(AbstractSyntaxTree));
    if (!node) return NULL;

    node->expression_type = BINARY_OP_EXPR;
    node->ExpressionUnion.operator_expr.operator_expr = operator_expr;
    node->ExpressionUnion.operator_expr.left = left;
    node->ExpressionUnion.operator_expr.right = right;

    return node;
}

/*************************************************************************************************************/
// "return return_value"

AbstractSyntaxTree* make_returnExpr(AbstractSyntaxTree* return_value) {
    AbstractSyntaxTree* node = malloc(sizeof(AbstractSyntaxTree));
    if (!node) return NULL;

    node->expression_type = RETURN_EXPR;
    node->ExpressionUnion.return_expr.return_expr = return_value;

    return node;
}

/*************************************************************************************************************/
// "while (condition) { while_body }"

AbstractSyntaxTree* make_whileExpr(AbstractSyntaxTree* while_condition, Vec* while_body) {
    AbstractSyntaxTree* node = malloc(sizeof(AbstractSyntaxTree));
    if (!node) return NULL;

    node->expression_type = WHILE_EXPR;
    node->ExpressionUnion.while_expr.while_condition = while_condition;
    node->ExpressionUnion.while_expr.while_body = while_body;

    return node;
}

/*************************************************************************************************************/
// "if (condition) { if_body } else { else _body }"

AbstractSyntaxTree* make_conditionExpr(AbstractSyntaxTree* if_condition, Vec* if_body, Vec* else_body) {
    AbstractSyntaxTree* node = malloc(sizeof(AbstractSyntaxTree));
    if (!node) return NULL;

    node->expression_type = CONDITIONS_EXPR;
    node->ExpressionUnion.conditions_expr.if_condition = if_condition;
    node->ExpressionUnion.conditions_expr.if_body = if_body;
    node->ExpressionUnion.conditions_expr.else_body = else_body;

    return node;
}

/*************************************************************************************************************/
// "... func_name(params)"

AbstractSyntaxTree* make_function_callExpr(String func_name, Vec* func_arguments) {
    AbstractSyntaxTree* node = malloc(sizeof(AbstractSyntaxTree));
    if (!node) return NULL;

    node->expression_type = FUNCTION_CALL_EXPR;
    node->ExpressionUnion.func_call_expr.func_name = func_name;
    node->ExpressionUnion.func_call_expr.func_arguments = func_arguments;

    return node;
}

/*************************************************************************************************************/
// "... pom(params) { func_body }"

AbstractSyntaxTree* make_function_defExpr(String func_name, Vec* func_parameters, Vec* func_body, enum ValueType ret_val) {
    AbstractSyntaxTree* node = malloc(sizeof(AbstractSyntaxTree));
    if (!node) return NULL;

    node->expression_type = FUNCTION_DEF_EXPR;
    node->ExpressionUnion.func_def_expr.func_name = func_name;
    node->ExpressionUnion.func_def_expr.func_parameters = func_parameters;
    node->ExpressionUnion.func_def_expr.func_body = func_body;
    node->ExpressionUnion.func_def_expr.ret_val = ret_val;

    return node;
}

/*************************************************************************************************************/
// "... pom : String = right_part"

AbstractSyntaxTree* make_variable_Expr(String var_name, enum ValueType type, AbstractSyntaxTree* right_part) {
    AbstractSyntaxTree* node = malloc(sizeof(AbstractSyntaxTree));
    if (!node) return NULL;

    node->expression_type = VARIABLE_DEC_EXPR;
    node->ExpressionUnion.var_expr.name = var_name;
    node->ExpressionUnion.var_expr.VarUnion.value.type = type;
    node->ExpressionUnion.var_expr.VarUnion.right_part = right_part;

    return node;
}

/*************************************************************************************************************/
// "...(value)", where value is f.e. 5 ALSO use for creating

AbstractSyntaxTree* make_valueExpr(String arg_name, enum ValueType type, Lexer* lex) {
    AbstractSyntaxTree* node = malloc(sizeof(AbstractSyntaxTree));
    if (!node) return NULL;

    node->expression_type = VARIABLE_VALUE;
    node->ExpressionUnion.var_expr.name = arg_name;
    node->ExpressionUnion.var_expr.VarUnion.value.type = type;

    switch (type) {
        case INT_VALUE:
            node->ExpressionUnion.var_expr.VarUnion.value.ValueUnion.int_value = lex->i_num;
            break;
        case DOUBLE_VALUE:
            node->ExpressionUnion.var_expr.VarUnion.value.ValueUnion.double_value = lex->d_num;
            break;
        default: // STRING VALUE
            node->ExpressionUnion.var_expr.VarUnion.value.ValueUnion.string_value = lex->str;
            break;
    }

    return node;
}

/*************************************************************************************************************/
// Parser creates tree for each param and stores it into vector
// "with y : String"

AbstractSyntaxTree* make_parameterExpr(String param_name, String param_ident, enum ValueType param_type) {
    AbstractSyntaxTree* node = malloc(sizeof(AbstractSyntaxTree));
    if (!node) return NULL;

    node->expression_type = FUNCTION_PARAM_EXPR;
    node->ExpressionUnion.parameter_expr.param_name = param_name;
    node->ExpressionUnion.parameter_expr.param_ident = param_ident;
    node->ExpressionUnion.parameter_expr.param_type = param_type;

    return node;
}

/*************************************************************************************************************/

/*
    Body will consists each lines, so the parser created tree for each line and stores it into vector
        => no need to have function for creating body tree
*/

/*************************************************************************************************************/
