#include "ast.h"

unsigned int i = 0;

void free_ast_list(ASTList* list) {
    if (list == NULL) {
        return;
    }

    //free_ast(list->node);
    free_ast_list(list->next);
    free(list);
}

void free_ast(AbstractSyntaxTree* node) {
    if (node == NULL) {
        return;
    }

    switch (node->expression_type) {
        case BINARY_OP_EXPR:
        case ASSIGN_EXPR:
            free_ast(node->ExpressionUnion.operator_expr.left);
            free_ast(node->ExpressionUnion.operator_expr.right);
            break;

        case FUNCTION_CALL_EXPR:
            free(node->ExpressionUnion.func_call_expr.func_call_name_expr);
            free_ast_list(node->ExpressionUnion.func_call_expr.func_arguments);
            break;

        case FUNCTION_DEF_EXPR:
            free(node->ExpressionUnion.func_def_expr.func_def_name_expr);
            free_ast_list(node->ExpressionUnion.func_def_expr.func_parameters);
            free_ast(node->ExpressionUnion.func_def_expr.func_body);
            break;

        case RETURN_EXPR:
            free_ast(node->ExpressionUnion.return_expr.return_expr);
            break;

        case CONDITIONS_EXPR:
        
            free_ast(node->ExpressionUnion.conditions_expr.if_condition);
            free_ast(node->ExpressionUnion.conditions_expr.if_body);
            free_ast(node->ExpressionUnion.conditions_expr.else_body);
            break;

        case WHILE_EXPR:
            free_ast(node->ExpressionUnion.while_expr.while_condition);
            free_ast(node->ExpressionUnion.while_expr.while_body);
            break;

        case VARIABLE_DEF_EXPR:
            free(node->ExpressionUnion.var_def_expr.var_def_name_expr);
            free_ast(node->ExpressionUnion.var_def_expr.variable_type);
            free_ast(node->ExpressionUnion.var_def_expr.variable_value);
            break;

        case VARIABLE_DEC_EXPR:
            free(node->ExpressionUnion.var_dec_expr.var_dec_name_expr);
            free_ast(node->ExpressionUnion.var_dec_expr.variable_type);
            free_ast(node->ExpressionUnion.var_dec_expr.variable_value);
            break;

        case FUNCTION_PARAM_EXPR:
            free_ast(node->ExpressionUnion.parameter_expr.param_name);
            free_ast(node->ExpressionUnion.parameter_expr.param_type);
            break;

        case FUNCTION_ARG_EXPR:
            free_ast(node->ExpressionUnion.argument_expr.arg_value);
            break;

        case BODY_EXPR:
            free_ast(node->ExpressionUnion.body_expr.body);

        default:
            break;
    }

    free(node);
}

void preorder_traversal(AbstractSyntaxTree* node) {
    if (node == NULL) {
        return;
    }

    switch (node->expression_type) {
        case INTEGER_TYPE_EXPR:
            printf("Integer Type\n");
            preorder_traversal(node->ExpressionUnion.integer_type);
            break;

        case DOUBLE_TYPE_EXPR:
            printf("Double Type\n");
            preorder_traversal(node->ExpressionUnion.double_type);
            break;

        case STRING_TYPE_EXPR:
            printf("String Type\n");
            preorder_traversal(node->ExpressionUnion.string_type);
            break;

        case VARIABLE_EXPR:
            printf("Variable Expression: %s\n", node->ExpressionUnion.variable_expr);
            break;

        case BINARY_OP_EXPR:
            printf("Binary Operation: %s\n", node->ExpressionUnion.operator_expr.operator_expr);
            preorder_traversal(node->ExpressionUnion.operator_expr.left);
            preorder_traversal(node->ExpressionUnion.operator_expr.right);
            break;

        case RETURN_EXPR:
            printf("Return expression\n");
            preorder_traversal(node->ExpressionUnion.return_expr.return_expr);
            break;

        case ASSIGN_EXPR:
            printf("Assing expression: %s\n", node->ExpressionUnion.assign_expr.assign_expr);
            preorder_traversal(node->ExpressionUnion.assign_expr.left);
            preorder_traversal(node->ExpressionUnion.assign_expr.right);
            break;
        
        case WHILE_EXPR:
            printf("While expression\n");
            printf("While condition\n");
            preorder_traversal(node->ExpressionUnion.while_expr.while_condition);
            printf("While body\n");
            preorder_traversal(node->ExpressionUnion.while_expr.while_body);
            break;

        case CONDITIONS_EXPR:
            printf("Condition expression\n");
            printf("If condition\n");
            preorder_traversal(node->ExpressionUnion.conditions_expr.if_condition);
            printf("If body\n");
            preorder_traversal(node->ExpressionUnion.conditions_expr.if_body);
            printf("Else body\n");
            preorder_traversal(node->ExpressionUnion.conditions_expr.else_body);
            break;

        default:
            switch (node->ExpressionUnion.variable_value.type)
            {
            case INT_VALUE:
                printf("Integer Expression: %d\n", node->ExpressionUnion.variable_value.value.int_value);
                break;
                
            case DOUBLE_VALUE:
                printf("Double Expression: %f\n", node->ExpressionUnion.variable_value.value.double_value);
                break;

            case STRING_VALUE:
                printf("String Expression: %s\n", node->ExpressionUnion.variable_value.value.string_value);
                break;   

            default:
                break;
            }
            break;
    }
}

// není kompletní - neuvolní celý strom
void node_fail(AbstractSyntaxTree* node) {
    if (node == NULL) {
        EPRINTF("Malloc fail\n");
        free_ast(node); // the functionality is quite weird, because when malloc fails to allocate 
                        // memory for your tree, what this function should free with pointer pointing to NULL address?
        exit(EXIT_FAILURE);
    }
}

AbstractSyntaxTree* make_binaryExpr(char* operator_expr, AbstractSyntaxTree* left, AbstractSyntaxTree* right) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*) malloc(sizeof(AbstractSyntaxTree));
    node_fail(node);

    node->expression_type = BINARY_OP_EXPR;
    node->ExpressionUnion.operator_expr.operator_expr = operator_expr;
    node->ExpressionUnion.operator_expr.left = left;
    node->ExpressionUnion.operator_expr.right = right;
    i++;
    return node;
}

AbstractSyntaxTree* make_variableExpr(char* variable_name) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*) malloc(sizeof(AbstractSyntaxTree));
    node_fail(node);

    node->expression_type = VARIABLE_EXPR;
    node->ExpressionUnion.variable_expr = variable_name;
    i++;
    return node;
}

AbstractSyntaxTree* make_integerExpr(int value) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*) malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);
    node->expression_type = VARIABLE_VALUE;
    node->ExpressionUnion.variable_value.type = INT_VALUE;
    node->ExpressionUnion.variable_value.value.int_value = value;

    i++;
    return node;
}

AbstractSyntaxTree* make_integerType(AbstractSyntaxTree* value) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*) malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = INTEGER_TYPE_EXPR;
    node->ExpressionUnion.integer_type = value;
    i++;
    return node;
}

AbstractSyntaxTree* make_doubleExpr(double value) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*) malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = VARIABLE_VALUE;
    node->ExpressionUnion.variable_value.type = DOUBLE_VALUE;
    node->ExpressionUnion.variable_value.value.double_value = value;
    i++;
    return node;
}

AbstractSyntaxTree* make_doubleType(AbstractSyntaxTree* value) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*) malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = DOUBLE_TYPE_EXPR;
    node->ExpressionUnion.double_type = value;
    i++;
    return node;
}

AbstractSyntaxTree* make_stringExpr(char* value) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*) malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = VARIABLE_VALUE;
    node->ExpressionUnion.variable_value.type = STRING_VALUE;
    node->ExpressionUnion.variable_value.value.string_value = value;
    i++;
    return node;
}

AbstractSyntaxTree* make_stringType(AbstractSyntaxTree* value) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*) malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = STRING_TYPE_EXPR;
    node->ExpressionUnion.string_type = value;
    i++;
    return node;
}

AbstractSyntaxTree* make_returnExpr(AbstractSyntaxTree* return_value) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*) malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = RETURN_EXPR;
    node->ExpressionUnion.return_expr.return_expr = return_value;
    i++;
    return node;
}

AbstractSyntaxTree* make_assignExpr(char* assign_expr, AbstractSyntaxTree* left, AbstractSyntaxTree* right) { // Potřebujeme?
    AbstractSyntaxTree* node = (AbstractSyntaxTree*) malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = ASSIGN_EXPR;
    node->ExpressionUnion.assign_expr.assign_expr = assign_expr;
    node->ExpressionUnion.assign_expr.left = left;
    node->ExpressionUnion.assign_expr.right = right;
    i++;
    return node;
}

AbstractSyntaxTree* make_whileExpr(AbstractSyntaxTree* while_condition, AbstractSyntaxTree* while_body) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*) malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = WHILE_EXPR;
    node->ExpressionUnion.while_expr.while_condition = while_condition;
    node->ExpressionUnion.while_expr.while_body = while_body;
    i++;
    return node;
}

AbstractSyntaxTree* make_conditionExpr(AbstractSyntaxTree* if_condition, AbstractSyntaxTree* if_body, AbstractSyntaxTree* else_body) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*) malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);
    
    node->expression_type = CONDITIONS_EXPR;
    node->ExpressionUnion.conditions_expr.if_condition = if_condition;
    node->ExpressionUnion.conditions_expr.if_body = if_body;
    node->ExpressionUnion.conditions_expr.else_body = else_body;
    i++;
    return node;
}

AbstractSyntaxTree* make_function_callExpr(char* func_call_name_expr, ASTList* func_arguments) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*)malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = FUNCTION_CALL_EXPR;
    node->ExpressionUnion.func_call_expr.func_call_name_expr = func_call_name_expr;
    node->ExpressionUnion.func_call_expr.func_arguments = func_arguments;
    i++;
    return node;
}

AbstractSyntaxTree* make_function_defExpr(char* func_def_name_expr, ASTList* func_parameters, AbstractSyntaxTree* func_body) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*)malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = FUNCTION_DEF_EXPR;
    node->ExpressionUnion.func_def_expr.func_def_name_expr = func_def_name_expr;
    node->ExpressionUnion.func_def_expr.func_parameters = func_parameters;
    node->ExpressionUnion.func_def_expr.func_body = func_body;
    i++;
    return node;
}

AbstractSyntaxTree* make_variable_defExpr(char* var_def_name_expr, AbstractSyntaxTree* varible_type, AbstractSyntaxTree* variable_value) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*)malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = VARIABLE_DEF_EXPR;
    node->ExpressionUnion.var_def_expr.var_def_name_expr = var_def_name_expr;
    node->ExpressionUnion.var_def_expr.variable_type = varible_type;
    node->ExpressionUnion.var_def_expr.variable_value = variable_value;
    i++;
    return node;
}

AbstractSyntaxTree* make_variable_decExpr(char* var_dec_name_expr, AbstractSyntaxTree* varible_type, AbstractSyntaxTree* variable_value) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*)malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = VARIABLE_DEC_EXPR;
    node->ExpressionUnion.var_dec_expr.var_dec_name_expr = var_dec_name_expr;
    node->ExpressionUnion.var_dec_expr.variable_type = varible_type;
    node->ExpressionUnion.var_dec_expr.variable_value = variable_value;
    i++;
    return node;
}

AbstractSyntaxTree* make_ArgumentExpr(AbstractSyntaxTree* arg_name) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*)malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = FUNCTION_ARG_EXPR;
    node->ExpressionUnion.argument_expr.arg_value = arg_name;
    i++;
    return node;
}

AbstractSyntaxTree* make_ParameterExpr(AbstractSyntaxTree* param_name, AbstractSyntaxTree* param_type, ASTList* next) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*)malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = FUNCTION_PARAM_EXPR;
    node->ExpressionUnion.parameter_expr.param_name = param_name;
    node->ExpressionUnion.parameter_expr.param_type = param_type;
    i++;
    return node;
}

AbstractSyntaxTree* make_BodyExpr(AbstractSyntaxTree* body, ASTList* next) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*)malloc(sizeof(AbstractSyntaxTree));
    node_fail(node);

    node->expression_type = BODY_EXPR;
    node->ExpressionUnion.body_expr.body = body;
    node->ExpressionUnion.body_expr.next = next;

    return node;
}







int main(void) {

    //if (a<2) {
    //b = b + 1;
    //a = a + 1;
    //}
    //else {
    //return 0
    //}  


   /* AbstractSyntaxTree* if_condition = make_binaryExpr("<", make_variableExpr("a"), make_integerExpr(2));

    AbstractSyntaxTree* if_body = make_BodyExpr(
        make_assignExpr("=", make_variableExpr("b"), make_binaryExpr("+", make_variableExpr("b"), make_integerExpr(1))),
            NULL
    );

    AbstractSyntaxTree* else_body = make_BodyExpr(
        make_returnExpr(make_integerExpr(0)),
        NULL
    );


    AbstractSyntaxTree* root = make_conditionExpr(if_condition, if_body, else_body);*/

/* AbstractSyntaxTree* param1 = make_ParameterExpr(make_variableExpr("ahoj"), make_integerType(make_integerExpr(1)), NULL);

AbstractSyntaxTree* param2 = make_ParameterExpr(make_variableExpr("ahoj"), make_integerType(make_integerExpr(1)), param1);

ASTList* param = (ASTList*) param2;

AbstractSyntaxTree* root = make_function_defExpr("funkce", param2, make_binaryExpr("<", make_variableExpr("a"), make_integerExpr(2))); */


    printf("Number of NODES: %d\n", i);
    printf("\nPOSTORDER TRAVERSAL:\n");
   // preorder_traversal(root);
   // free_ast(root);
}