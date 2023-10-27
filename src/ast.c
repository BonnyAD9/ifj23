#include "ast.h"

int i = 0;

void free_ast_list(ASTList* list) {
    if (list == NULL) {
        return;
    }

    free_ast(&list->node);
    free_ast_list(list->next);
    free(list);
}

void free_ast(AbstractSyntaxTree* node) {
    if (node == NULL) {
        return;
    }

    switch (node->expression_type) {
        case INTEGER_TYPE_EXPR:
        case DOUBLE_TYPE_EXPR:
        case STRING_TYPE_EXPR:
        case INTEGER_EXPR:
        case DOUBLE_EXPR:
        case STRING_EXPR:
        case VARIABLE_EXPR:
            break;

        case BINARY_OP_EXPR:
        case ASSIGN_EXPR:
            free_ast(node->ExpressionUnion.OperatorExpr.left);
            free_ast(node->ExpressionUnion.OperatorExpr.right);
            break;

        case FUNCION_CALL_EXPR:
            free(node->ExpressionUnion.FunctionCallExpr.func_call_name_expr);
            free_ast_list(node->ExpressionUnion.FunctionCallExpr.func_arguments);
            break;

        case FUNCION_DEF_EXPR:
            free(node->ExpressionUnion.FunctionDefExpr.func_def_name_expr);
            free_ast_list(node->ExpressionUnion.FunctionDefExpr.func_parameters);
            free_ast(node->ExpressionUnion.FunctionDefExpr.func_body);
            break;

        case RETURN_EXPR:
            free_ast(node->ExpressionUnion.return_expr);
            break;

        case CONDITIONS_EXPR:
            free_ast(node->ExpressionUnion.ConditionsExpr.if_condition);
            free_ast(node->ExpressionUnion.ConditionsExpr.if_body);
            free_ast(node->ExpressionUnion.ConditionsExpr.else_body);
            break;

        case WHILE_EXPR:
            free_ast(node->ExpressionUnion.WhileExpr.while_condition);
            free_ast(node->ExpressionUnion.WhileExpr.while_body);
            break;

        case VARIABLE_DEF_EXPR:
            free(node->ExpressionUnion.VariableDefExpr.var_def_name_expr);
            free_ast(node->ExpressionUnion.VariableDefExpr.varible_type);
            free_ast(node->ExpressionUnion.VariableDefExpr.variable_value);
            break;

        case VARIABLE_DEC_EXPR:
            free(node->ExpressionUnion.VariableDecExpr.var_dec_name_expr);
            free_ast(node->ExpressionUnion.VariableDecExpr.varible_type);
            free_ast(node->ExpressionUnion.VariableDecExpr.variable_value);
            break;

        case FUNCTION_PARAM_EXPR:
            free_ast(node->ExpressionUnion.ParameterExpr.param_name);
            free_ast(node->ExpressionUnion.ParameterExpr.param_type);
            //free_ast((AbstractSyntaxTree*)node->ExpressionUnion.ParameterExpr.next);
            break;

        case FUNCTION_ARG_EXPR:
            free_ast(node->ExpressionUnion.ArgumentExpr.arg_value);
            //free_ast((AbstractSyntaxTree*)node->ExpressionUnion.ArgumentExpr.next);
            break;


        default:
            EPRINTF("Weird node - fail\n");
            break;
    }

    free(node);
}

// není zde vše, spíš to byla jen taková pomocná funkce, která mi pomáhala vizualizovat strom
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

        case INTEGER_EXPR:
            printf("Integer Expression: %d\n", node->ExpressionUnion.integer_expr);
            break;

        case DOUBLE_EXPR:
            printf("Double Expression: %f\n", node->ExpressionUnion.double_expr);
            break;

        case STRING_EXPR:
            printf("String Expression: %s\n", node->ExpressionUnion.string_expr);
            break;

        case VARIABLE_EXPR:
            printf("Variable Expression: %s\n", node->ExpressionUnion.variable_expr);
            break;

        case BINARY_OP_EXPR:
            printf("Binary Operation: %s\n", node->ExpressionUnion.OperatorExpr.operator_expr);
            preorder_traversal(node->ExpressionUnion.OperatorExpr.left);
            preorder_traversal(node->ExpressionUnion.OperatorExpr.right);
            break;

        case RETURN_EXPR:
            printf("Return expression\n");
            preorder_traversal(node->ExpressionUnion.return_expr);
            break;

        case ASSIGN_EXPR:
            printf("Assing expression: %s\n", node->ExpressionUnion.AssignExpr.assign_expr);
            preorder_traversal(node->ExpressionUnion.AssignExpr.left);
            preorder_traversal(node->ExpressionUnion.AssignExpr.right);
            break;
        
        case WHILE_EXPR:
            printf("While expression\n");
            printf("While condition\n");
            preorder_traversal(node->ExpressionUnion.WhileExpr.while_condition);
            printf("While body\n");
            preorder_traversal(node->ExpressionUnion.WhileExpr.while_body);
            break;

        case CONDITIONS_EXPR:
            printf("Condition expression\n");
            printf("If condition\n");
            preorder_traversal(node->ExpressionUnion.ConditionsExpr.if_condition);
            printf("If body\n");
            preorder_traversal(node->ExpressionUnion.ConditionsExpr.if_body);
            printf("Else body\n");
            preorder_traversal(node->ExpressionUnion.ConditionsExpr.else_body);
            break;

        default:
            printf("Neznámý typ uzlu\n");
            break;
    }
}

// není kompletní - neuvolní celý strom
AbstractSyntaxTree* node_fail(AbstractSyntaxTree* node) {
    if (node == NULL) {
        EPRINTF("Malloc fail\n");
        free_ast(node);
        exit(EXIT_FAILURE);
    }
}

AbstractSyntaxTree* make_binaryExpr(char* operator_expr, AbstractSyntaxTree* left, AbstractSyntaxTree* right) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*) malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = BINARY_OP_EXPR;
    node->ExpressionUnion.OperatorExpr.operator_expr = operator_expr;
    node->ExpressionUnion.OperatorExpr.left = left;
    node->ExpressionUnion.OperatorExpr.right = right;
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

    node->expression_type = INTEGER_EXPR;
    node->ExpressionUnion.integer_expr = value;
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

    node->expression_type = DOUBLE_EXPR;
    node->ExpressionUnion.double_expr = value;
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

    node->expression_type = STRING_EXPR;
    node->ExpressionUnion.string_expr = value;
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
    node->ExpressionUnion.string_type = return_value;
    i++;
    return node;
}

AbstractSyntaxTree* make_assignExpr(char* assign_expr, AbstractSyntaxTree* left, AbstractSyntaxTree* right) { // Potřebujeme?
    AbstractSyntaxTree* node = (AbstractSyntaxTree*) malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = ASSIGN_EXPR;
    node->ExpressionUnion.AssignExpr.assign_expr = assign_expr;
    node->ExpressionUnion.AssignExpr.left = left;
    node->ExpressionUnion.AssignExpr.right = right;
    i++;
    return node;
}

AbstractSyntaxTree* make_whileExpr(AbstractSyntaxTree* while_condition, AbstractSyntaxTree* while_body) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*) malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = WHILE_EXPR;
    node->ExpressionUnion.WhileExpr.while_condition = while_condition;
    node->ExpressionUnion.WhileExpr.while_body = while_body;
    i++;
    return node;
}

AbstractSyntaxTree* make_conditionExpr(AbstractSyntaxTree* if_condition, AbstractSyntaxTree* if_body, AbstractSyntaxTree* else_body) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*) malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = CONDITIONS_EXPR;
    node->ExpressionUnion.ConditionsExpr.if_condition = if_condition;
    node->ExpressionUnion.ConditionsExpr.if_body = if_body;
    node->ExpressionUnion.ConditionsExpr.else_body = else_body;
    i++;
    return node;
}

AbstractSyntaxTree* make_function_callExpr(char* func_call_name_expr, ASTList* func_arguments) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*)malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = FUNCION_CALL_EXPR;
    node->ExpressionUnion.FunctionCallExpr.func_call_name_expr = func_call_name_expr;
    node->ExpressionUnion.FunctionCallExpr.func_arguments = func_arguments;
    i++;
    return node;
}

AbstractSyntaxTree* make_function_defExpr(char* func_def_name_expr, ASTList* func_parameters, AbstractSyntaxTree* func_body) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*)malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = FUNCION_DEF_EXPR;
    node->ExpressionUnion.FunctionDefExpr.func_def_name_expr = func_def_name_expr;
    node->ExpressionUnion.FunctionDefExpr.func_parameters = func_parameters;
    node->ExpressionUnion.FunctionDefExpr.func_body = func_body;
    i++;
    return node;
}

AbstractSyntaxTree* make_variable_defExpr(char* var_def_name_expr, AbstractSyntaxTree* varible_type, AbstractSyntaxTree* variable_value) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*)malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = VARIABLE_DEF_EXPR;
    node->ExpressionUnion.VariableDefExpr.var_def_name_expr = var_def_name_expr;
    node->ExpressionUnion.VariableDefExpr.varible_type = varible_type;
    node->ExpressionUnion.VariableDefExpr.variable_value = variable_value;
    i++;
    return node;
}

AbstractSyntaxTree* make_variable_decExpr(char* var_dec_name_expr, AbstractSyntaxTree* varible_type, AbstractSyntaxTree* variable_value) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*)malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = VARIABLE_DEC_EXPR;
    node->ExpressionUnion.VariableDecExpr.var_dec_name_expr = var_dec_name_expr;
    node->ExpressionUnion.VariableDecExpr.varible_type = varible_type;
    node->ExpressionUnion.VariableDecExpr.variable_value = variable_value;
    i++;
    return node;
}

AbstractSyntaxTree* make_ArgumentExpr(AbstractSyntaxTree* arg_name) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*)malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = FUNCTION_ARG_EXPR;
    node->ExpressionUnion.ArgumentExpr.arg_value = arg_name;
    i++;
    return node;
}

AbstractSyntaxTree* make_ParameterExpr(AbstractSyntaxTree* param_name, AbstractSyntaxTree* param_type) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*)malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = FUNCTION_PARAM_EXPR;
    node->ExpressionUnion.ParameterExpr.param_name = param_name;
    node->ExpressionUnion.ParameterExpr.param_type = param_type;
    i++;
    return node;
}

AbstractSyntaxTree* make_BodyExpr(AbstractSyntaxTree* body, ASTList* next) {
    AbstractSyntaxTree* node = (AbstractSyntaxTree*)malloc(sizeof(AbstractSyntaxTree));

    node_fail(node);

    node->expression_type = BODY_EXPR;
    node->ExpressionUnion.BodyExpr.body = body;
    node->ExpressionUnion.BodyExpr.next = next;

    return node;
}



int main(void) {
    //AbstractSyntaxTree* node;
    // node = make_binaryExp("+", make_variableExp("x"), make_integerExp(5));
    //node = make_assignExpr("=", make_variableExpr("a"), make_integerExpr(5));
    
    //node = make_whileExpr(make_binaryExpr("<", make_variableExpr("a"), make_binaryExpr("+", make_variableExpr("i"), make_integerExpr(1))),
    //                    make_returnExpr(make_stringExpr("ahojky")));

                    // while (a < i + 1) {
                    //    return ahojky;
                    //}
    
    //AbstractSyntaxTree* node2;
    //node2 = make_conditionExpr(make_binaryExpr("<", make_variableExpr("a"),make_integerExpr(1)), 
    //                            make_binaryExpr("+", make_variableExpr("i"), make_integerExpr(1)),
     //                            make_binaryExpr("-", make_variableExpr("i"), make_integerExpr(1))
    //                           );
                    // if (a < 1) {
                    //      i + 1;
                    //   else {
                    //      i - 1;
                    //   }
                    //}

    //AbstractSyntaxTree* root;
    //ASTList* body = 

    //root = make_conditionExpr(make_binaryExpr("<", make_variableExpr("a"), make_integerExpr(2)),  //if (a<2) {

    
    //);


    //b = b + 1;
    //a = a + 1;
    //}
    //else {
    //return 0
    //}  




    printf("Number of NODES: %d\n", i);
    printf("\nPOSTORDER TRAVERSAL:\n");
    //preorder_traversal(root);
    //free_ast(node);
    //free_ast(node2);
    //free_ast(root);
}