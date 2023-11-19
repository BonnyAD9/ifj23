#include "semantics.h"

// TODO keep up-date
static bool    *sema_err(const char *msg, const int err_type);
static Context *new_context(bool in_func, bool in_if, bool in_main, bool in_while, AstFuncType func_type);
static bool     process_statement(AstStmt *stmt);
static bool     check_expr(AstExpr *expr);
static bool     process_expr(AstExpr *expr, AstDataType *final_type);
static bool     check_binary_op(AstBinaryOp *binary_op, AstDataType *final_type);
static bool     check_unary_op(AstUnaryOp *unary_op, AstDataType *final_type);
static bool     check_func_call(AstFunctionCall *func_call, AstDataType *final_type);
static bool     check_func_params(SymItem *ident, Vec params);
static bool     check_literal(AstLiteral *literal, AstDataType *final_type);
static bool     check_variable(SymItem *ident, AstDataType *final_type);
static bool     check_compatibility(Token operator, AstDataType left_type, AstDataType right_type, AstDataType *final_type);
static bool     check_block(AstBlock *block);
static bool     check_func_decl(AstFunctionDecl *function_decl, Context *context);
static bool     check_var_decl(AstVariableDecl *variable_decl);
static bool     check_return(AstReturn *return_v, Context *context);
static bool     check_if_stmt(AstIf *if_v);
static bool     check_while_stmt(AstWhile *while_v);

/////////////////////////////////////////////////////////////////////////
// Context type vec
Vec context_vec;
/////////////////////////////////////////////////////////////////////////

static bool *sema_err(const char *msg, const int err_type) {
    set_err_code(err_type);
    EPRINTF("%s", msg);
    return false;
}

static Context *new_context(bool in_func, bool in_if, bool in_main, bool in_while, AstFuncType func_type) {
    Context *cont = malloc(sizeof(Context));
    cont->in_func     = in_func;
    cont->in_if       = in_if;
    cont->in_main     = in_main;
    cont->in_while    = in_while;
    cont->return_used = false;
    cont->func_type   = func_type;
    return cont;
}

// Initial call from parser
bool check_statement(AstStmt *stmt) {
    context_vec = VEC_NEW(Context);
    // Set initial context and push it to vector
    Context *new_cont = new_context(false, false, true, false, UNKNOWN);
    VEC_PUSH(&context_vec, Context*, new_cont);

    bool retval = process_statement(stmt);
    // Free and clean created context vector
    free(new_cont);
    vec_free(&context_vec);

    return retval;
}

static bool process_statement(AstStmt *stmt) {
    Context *cur_context;
    switch (stmt->type) {
        case AST_EXPR:
            return check_expr(stmt->expr);
        case AST_BLOCK:
            return check_block(stmt->block);
        case AST_FUNCTION_DECL:
            cur_context = VEC_LAST(context_vec, Context*);
            if (cur_context->in_func || cur_context->in_if || cur_context->in_while) // Invalid combination
                return sema_err("Cant declare/define inside function", ERR_SEMANTIC);
            // New - in function - context
            Context *new_cont = new_context(true, false, false, false, UNKNOWN);
            VEC_PUSH(&context_vec, Context*, new_cont);
            bool retval = check_func_decl(stmt->function_decl, new_cont);

            // Check for missing return statement
            if (new_cont->func_type != VOID_FUNC && !new_cont->return_used)
                return sema_err("Missing return statement in function", ERR_INVALID_RETURN);

            // Free before poping
            free(new_cont);
            // Pop exiting context when check done
            VEC_POP(&context_vec, Context);

            return retval;
        case AST_VARIABLE_DECL:
            return check_var_decl(stmt->variable_decl);
        case AST_RETURN:
            cur_context = VEC_LAST(context_vec, Context*);
            if (cur_context->in_main && !cur_context->in_func) // Cant do return from main body
                return sema_err("Return in main body", ERR_SEMANTIC);
            // In function
            return check_return(stmt->return_v, cur_context);
        case AST_IF:
            cur_context = VEC_LAST(context_vec, Context*);
            cur_context->in_if = true;

            return check_if_stmt(stmt->if_v);
        case AST_WHILE:
            cur_context = VEC_LAST(context_vec, Context*);
            cur_context->in_while = true;

            return check_while_stmt(stmt->while_v);
        default:
            return sema_err("Uknown stament type", ERR_SEMANTIC);
    }
}

/////////////////////////////////////////////////////////////////////////
// avoid f.e. having func call on left side of expression or if statement and so on

static bool check_expr(AstExpr *expr) {
    AstDataType type;
    return process_expr(expr, &type);
}

static bool process_expr(AstExpr *expr, AstDataType *final_type) {
    switch (expr->type) {
        case AST_BINARY_OP:
            return check_binary_op(expr->binary_op, final_type);
        case AST_UNARY_OP:
            return check_unary_op(expr->unary_op, final_type);
        case AST_FUNCTION_CALL:
            return check_func_call(expr->function_call, final_type);
        case AST_LITERAL:
            return check_literal(expr->literal, final_type);
        case AST_VARIABLE:
            return check_variable(expr->variable->ident, final_type);
        default:
            return sema_err("Uknown expression type", ERR_SEMANTIC);
    }
}

static bool check_binary_op(AstBinaryOp *binary_op, AstDataType *final_type) {
    // Avoid f.e. funccall(a,b) = 5 type of expressions
    AstExprType left_expr_type = binary_op->left->type;
    if ((left_expr_type == AST_FUNCTION_CALL || left_expr_type == AST_LITERAL) && binary_op->operator == '=')
        return sema_err("Invalid l-value of expression", ERR_SEMANTIC);

    bool left_side = process_expr(binary_op->left, final_type);
    AstDataType left_side_type = final_type;

    bool right_side = process_expr(binary_op->right, final_type);
    AstDataType right_side_type = final_type;

    return (left_side && right_side && check_compatibility(binary_op->operator, left_side_type, right_side_type, final_type));
}

static bool check_unary_op(AstUnaryOp *unary_op, AstDataType *final_type) {
    bool eval_expr = process_expr(unary_op->param, final_type);
    // Check for non-translateable options
    if (*final_type == UNKNOWN || *final_type == NOT_DEFINED)
        return sema_err("Invalid expression type, cant convert to non-nil type", ERR_SEMANTIC);

    // Only allowed unary operator is '!'
    if (unary_op->operator != '!')
        return sema_err("Invalid unary operator", ERR_SEMANTIC);

    // Switch to NOT_NIL values if not already set
    // NOT_NIL values are on even (sude) places in enum
    if (*final_type % 2)
        *final_type++;
    // Otherwise already set to NON-NIL value
    return eval_expr;
}

static bool check_func_call(AstFunctionCall *func_call, AstDataType *final_type) {
    // If no return, f.e. NO_RET type will be stored in
    bool func_exists = get_type_from_table(func_call->ident, final_type);
    if (!func_exists)
        return sema_err("Undefined function", ERR_UNDEF_FUNCTION);
    return check_func_params(func_call->ident, func_call->arguments);
}

static bool check_func_params(SymItem *ident, Vec args) {
    // TODO make sure symtable will store it as expected vector
    Vec *params = get_params_vector_for_function(ident);
    if (!params)
        return sema_err("Error while retrieving function params", ERR_SEMANTIC);
    // Check for same vector length
    if (params->len != args.len)
        return sema_err("Provided count of args differs from func's count of params", ERR_SEMANTIC);

    /*
        func decrement(of n: Int, by m: Int) -> Int { [params]
            return n - m
        }
        let decremented_n = decrement(of: n, by: 1)   [arguments]
    */
    VEC_FOR_EACH(&(args), AstFuncCallParam, arg) {
        AstFuncDeclParam param = VEC_AT(params, AstFuncDeclParam, arg.i);
        AstDataType param_data_type, arg_data_type;

        // Load also param data type
        check_variable(param.name, &param_data_type);

        if (arg.v->arg_type == VARIABLE) {
            // Check if variable is defined
            if (!check_variable(arg.v->name, &arg_data_type))
                // Error already set by check_variable()
                return false;
        }
        // Check for correct ident name
        // TODO when symtable done, return var's name to compare + compare scopes
        if (arg.v->ident != param.ident)
            return sema_err("Expected func argument differs in ident name", ERR_INVALID_FUN);

        // Check if correct type
        // f.e. passing NIL to Int? is valid
        if (param_data_type != arg_data_type) {
            if ((param_data_type % 2) && arg_data_type != LITERAL_NIL) // LITERAL_INT, LITERAL_DOUBLE, LITERAL_STRING, LITERAL_NIL
                return sema_err("Uncompatible argument and parameter type", ERR_INVALID_FUN);
            if (!(param_data_type % 2) && arg_data_type != LITERAL_NIL_NOT_NIL) // LITERAL_INT_NOT_NIL, LITERAL_DOUBLE_NOT_NIL, LITERAL_STRING_NOT_NIL, LITERAL_NIL_NOT_NIL
                return sema_err("Uncompatible argument and parameter type", ERR_INVALID_FUN);
        }
    }
    return true;
}

static bool check_literal(AstLiteral *literal, AstDataType *final_type) {
    // Implicitni konverze literalu na DOUBLE typ
    if (literal->data_type == LITERAL_INT_NOT_NIL)
        *final_type = LITERAL_DOUBLE_NOT_NIL;
    else
        *final_type = literal->data_type;
    return true;
}

static bool check_variable(SymItem *ident, AstDataType *final_type) {
    if (get_type_from_table(ident, final_type)) {
        if (final_type == LITERAL_INT || final_type == LITERAL_DOUBLE || final_type == LITERAL_STRING)
            // Throw warning about potencial nil
            WPRINTF("Variable can contain nil value");
        return true;
    }
    return sema_err("Undefined variable", ERR_UNDEF_FUNCTION);
}

/*
    Operation:
        + ... Int + Int ; Int + Double ; Double + Int ; String + String (pokud Double tak prevod Int->Double)
        - ... Int - Int ; Int - Double ; Double - Int ; Double - Double (--||--)
        * ... Int * Int ; Int * Double ; Double * Int ; Double * Double (--||--)
        / ... Int / Int ; Double / Double
        ==, !=, >,<, <=, => ... must be same type or error 7 (+ for == and != if nil, we need to convert it to non-nil value)
*/
static bool check_compatibility(Token operator, AstDataType left_type, AstDataType right_type, AstDataType *final_type) {
    const char *err_msg = NULL;
    switch (operator) {
        case '*':
        case '-':
        case '/':
            if (!compat_array[0][left_type - 1][right_type - 1]) {
                err_msg = "Uncompatible types for performing ";
                strcat(err_msg, ((operator == '*') ? "'*' operation" : (operator == '-') ? "'-' operation" : "'/' operation"));
            }
            else
                *final_type = final_type_arr[left_type - 1][right_type - 1];
            break;
        case '+':
            if (!compat_array[1][left_type - 1][right_type - 1])
                err_msg = "Uncompatible types for performing '+' operation";
            else
                *final_type = final_type_arr[left_type - 1][right_type - 1];
            break;

        case T_EQUALS:           // '=='
        case T_DIFFERS:          // '!='
        case T_LESS_OR_EQUAL:    // '<='
        case T_GREATER_OR_EQUAL: // '>='
            if (!compat_array[2][left_type - 1][right_type - 1]) {
                err_msg = "Uncompatible types for performing ";
                strcat(err_msg, ((operator == T_EQUALS) ? "'==' operation" : (operator == T_DIFFERS) ? "'!=' operation" : (operator == T_LESS_OR_EQUAL) ? "'<=' operation" : "'>=' operation"));
            }
            else
                *final_type = final_type_arr[left_type - 1][right_type - 1];
            break;

        case T_DOUBLE_QUES:  // ??
            if (!compat_array[3][left_type - 1][right_type - 1])
                err_msg = "Uncompatible types for performing '??' operation";
            else
                *final_type = final_type_arr[left_type - 1][right_type - 1];
            break;

        case '=':
            if (!compat_array[4][left_type - 1][right_type - 1])
                err_msg = "Uncompatible types for performing '=' operation";
            else
                *final_type = final_type_arr[left_type - 1][right_type - 1];
            break;

        case '<':
        case '>':
            if (!compat_array[5][left_type - 1][right_type - 1]) {
                err_msg = "Uncompatible types for performing ";
                strcat(err_msg, ((operator == '<') ? "'<' operation" : "'>' operation"));
            }
            else
                *final_type = final_type_arr[left_type - 1][right_type - 1];
            break;

        default:
            err_msg = "Uknown operator type";
            break;
    }

    // If set, output error along with error message
    if (err_msg)
        return sema_err(err_msg, ERR_INCOM_TYPE);
    return true;
}

/////////////////////////////////////////////////////////////////////////

static bool check_block(AstBlock *block) {
    VEC_FOR_EACH(&(block->stmts), AstStmt, stmt) {
        if (!process_statement(stmt.v))
            return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////

static bool check_func_decl(AstFunctionDecl *function_decl, Context *context) {
    // Check if ident is defined
    bool func_exists = get_type_from_table(function_decl->ident, context->func_type);
    if (!func_exists)
        return sema_err("Undefined function", ERR_UNDEF_FUNCTION);

    // Check params
    return check_block(function_decl->body);
}

/////////////////////////////////////////////////////////////////////////

static bool check_var_decl(AstVariableDecl *variable_decl) {
    // Check ident is valid
    AstDataType var_type;
    bool var_exists = check_variable(variable_decl->ident, &var_type);
    if (!var_exists)
        return false;

    // Check expression
    if (variable_decl->value) {
        AstDataType expr_type;
        bool check_expr = process_expr(variable_decl->value, &expr_type);

        // Check if expr type is inferiable
        if (expr_type == LITERAL_NIL || !check_expr)
            return sema_err("Expression type cant be determined", ERR_UNKNOWN_TYPE);

        // Check if data type is given, or try determine it
        if (var_type == NOT_DEFINED)
            variable_decl->data_type = expr_type;
        else // var_type value is useless, just avoid using NULL to avoid NULL checks in the function
            return check_compatibility('=', var_type, expr_type, &var_type);
    }
    // No init expression, PARSER makes sure type is given
    return true;
}

/////////////////////////////////////////////////////////////////////////

static bool check_return(AstReturn *return_v, Context *context) {
    // maybe use global variable to for current context or pass it as another param
    AstDataType expr_type;
    // Void function with return expression is wrong
    if (context->func_type == VOID_FUNC && return_v->expr)
        return sema_err("Void funtion returns something", ERR_INVALID_RETURN);
    // Non-void function missing return expression
    if (context->func_type != VOID_FUNC && !return_v->expr)
        return sema_err("Missing function's return type", ERR_INVALID_RETURN);

    bool valid_expr = process_expr(return_v->expr, &expr_type);
    // Different return types
    if (expr_type != context->func_type || !valid_expr)
        return sema_err("Function returns different type than expected", ERR_INVALID_FUN);

    context->return_used = true;
    return valid_expr;
}

/////////////////////////////////////////////////////////////////////////

static bool check_if_stmt(AstIf *if_v) {
    bool valid_cond;
    if (if_v->condition->type == AST_COND_EXPR)
        valid_cond = check_expr(if_v->condition->expr);
    else if (if_v->condition->type == AST_COND_LET)
        valid_cond = check_var_decl(if_v->condition->let);
    // check if body
    bool valid_if_body = check_block(if_v->if_body);
    if (if_v->else_body) {
        bool valid_else_body = check_block(if_v->else_body);
        return valid_cond && valid_if_body && valid_else_body;
    }
    return valid_cond && valid_if_body;
}

/////////////////////////////////////////////////////////////////////////

static bool check_while_stmt(AstWhile *while_v) {
    bool valid_cond;
    if (while_v->condition->type == AST_COND_EXPR)
        valid_cond = check_expr(while_v->condition->expr);
    else if (while_v->condition->type == AST_COND_LET)
        valid_cond = check_var_decl(while_v->condition->let);
    bool valid_body = check_block(while_v->body);
    return valid_cond && valid_body;
}

/////////////////////////////////////////////////////////////////////////

static bool get_type_from_table(SymItem *ident, AstDataType *final_type) {
    // TODO FOR MARTIN, kdyz void funcke, aby to bylo ve finaltype rozpoznatelne
    return 0;
}

static Vec *get_params_vector_for_function(SymItem *funcname) {
    // TODO FOR MARTIN
    return NULL;
}

/////////////////////////////////////////////////////////////////////////
