#include "semantics.h"

// TODO write static func declarations after done

static bool *sema_err(const char *msg, const int err_type);

/////////////////////////////////////////////////////////////////////////

static bool *sema_err(const char *msg, const int err_type) {
    set_err_code(err_type);
    EPRINTF("%s", msg);
    return false;
}

bool check_statement(AstStmt *stmt) {
    Context context = {
        .in_func = false,
        .in_if = false,
        .in_main = false,
        .in_while = false,
        .func_type = UNKNOWN
    };
    return process_statement(stmt, &context);
}

bool process_statement(AstStmt *stmt, Context *context) {
    context->in_main = true;

    switch (stmt->type) {
        case AST_EXPR:
            return check_expr(stmt->expr);
        case AST_BLOCK:
            return check_block(stmt->block);
        case AST_FUNCTION_DECL:
            if (context->in_func || context->in_if || context->in_while) // Invalid combination
                return sema_err("Cant declare/define inside function", ERR_SEMANTIC);
            context->in_func = true;
            context->in_main = false;
            context->in_if = false;
            context->in_while = false;
            return check_func_decl(stmt->function_decl, context);
        case AST_VARIABLE_DECL:
            return check_var_decl(stmt->variable_decl);
        case AST_RETURN:
            if (context->in_main) // Cant do return from main body
                return sema_err("Return in main body", ERR_SEMANTIC);
            return check_return(stmt->return_v, context);
        case AST_IF:
            context->in_if = true;
            return check_if_stmt(stmt->if_v);
        case AST_WHILE:
            context->in_while = true;
            return check_while_stmt(stmt->while_v);
        default:
            return sema_err("Uknown stament type", ERR_SEMANTIC);
    }
}

/////////////////////////////////////////////////////////////////////////
// avoid f.e. having func call on left side of expression or if statement and so on

static bool check_expr(AstExpr *expr) {
    AstType type;
    return process_expr(expr, &type);
}

static bool process_expr(AstExpr *expr, AstType *final_type) {
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

static bool check_binary_op(AstBinaryOp *binary_op, AstType *final_type) {
    // Avoid f.e. funccall(a,b) = 5 type of expressions
    AstType type = binary_op->left->type;
    if ((type == AST_FUNCTION_CALL || type == AST_LITERAL) && binary_op->operator == '=')
        return sema_err("Invalid l-value of expression", ERR_SEMANTIC);

    bool left_side = process_expr(binary_op->left, final_type);
    AstType left_side_type = final_type;

    bool right_side = process_expr(binary_op->right, final_type);
    AstType right_side_type = final_type;

    return (left_side && right_side && check_compatibility(binary_op->operator, left_side_type, right_side_type, final_type));
}

static bool check_unary_op(AstUnaryOp *unary_op, AstType *final_type) {
    bool eval_expr = process_expr(unary_op->param, final_type);
    AstType eval_type = final_type;
    // Only allowed unary operator is '!'
    if (unary_op->operator != '!')
        return sema_err("Invalid unary operator", ERR_SEMANTIC);
    // TODO discuss what can be unary operator and how can behave + if also do checks like in binary_op about l-values
    return eval_expr;
}

static bool check_func_call(AstFunctionCall *func_call, AstType *final_type) {
    // If no return, f.e. NO_RET type will be stored in
    bool func_exists = get_type_from_table(func_call->ident, final_type);
    if (!func_exists)
        return sema_err("Undefined function", ERR_UNDEF_FUNCTION);
    return check_func_params(func_call->ident, func_call->arguments);
}

static bool check_func_params(SymItem *ident, Vec params) {
    // Check func's params
    // TODO funcname(ident : name, ident : 1) => we need to know if argument is variable or literal
    // TODO make sure symtable will store it as expected vector
    Vec *arg_vec = get_args_vector_for_function(ident);
    if (!arg_vec)
        return sema_err("Error while retrieving function args", ERR_SEMANTIC);
    // Check for same vector lenght
    if (arg_vec->len != params.len)
        return sema_err("Provided count of args differs from func's count of params", ERR_SEMANTIC);

    /*
        func decrement(of n: Int, by m: Int) -> Int {
            return n - m
        }
        let decremented_n = decrement(of: n, by: 1)
    */
    VEC_FOR_EACH(&(params), AstFuncCallParam, arg) {
        if (arg.v->type == VARIABLE) {
            // + check if defined variable is passed to func
            if (!check_variable(arg.v->name, NULL))
                // Error already set
                return false;
        }

        // Check for correct ident name
        if (arg.v->ident.name != VEC_AT(arg_vec, AstFuncDeclParam, arg.i).ident.name)
            return sema_err("Expected func argument differs in ident name", ERR_INVALID_FUN);

        // Check if correct type
        if (arg.v->ident.type != VEC_AT(arg_vec, AstFuncDeclParam, arg.i).ident.type)
            return sema_err("Expected func argument differs in data type", ERR_INVALID_FUN);
    }
}

static bool check_literal(AstLiteral *literal, AstType *final_type) {
    *final_type = literal->type;
    return true;
}

static bool check_variable(SymItem *ident, AstType *final_type) {
    if (get_type_from_table(ident, final_type))
        return true;
    return sema_err("Undefined variable", ERR_UNDEF_VAR);
}

/*
    Operation:
        + ... Int + Int ; Int + Double ; Double + Int ; String + String (pokud Double tak prevod Int->Double)
        - ... Int - Int ; Int - Double ; Double - Int ; Double - Double (--||--)
        * ... Int * Int ; Int * Double ; Double * Int ; Double * Double (--||--)
        / ... Int / Int ; Double / Double
        ==, !=, >,<, <=, => ... must be same type or error 7 (+ for == and != if nil, we need to convert it to non-nil value)
*/

static bool check_compatibility(Token operator, AstType left_type, AstType right_type, AstType *final_type) {
    const char *err_msg = NULL;
    switch (operator) {
        case '*':
        case '-':
            if (left_type == LITERAL_STRING || left_type == LITERAL_NIL || right_type == LITERAL_STRING || right_type == LITERAL_NIL)
                err_msg = "Cant multiply/substract from string/nil value";
            // Determine type of whole expression
            else  // If expression contains at least one double type, int values must be converted to double and whole expr will be double type
                *final_type = ((left_type == LITERAL_DOUBLE || right_type == LITERAL_DOUBLE) ? LITERAL_DOUBLE : LITERAL_INT);
            break;
        case '/':
            if (left_type != right_type || (left_type != LITERAL_INT && left_type != LITERAL_DOUBLE))
                err_msg = "Divided values must have same type";
            // Determine type of whole expression
            else
                *final_type = ((left_type == LITERAL_DOUBLE) ? LITERAL_DOUBLE : LITERAL_INT);
            break;
        case '+':
            if (left_type == LITERAL_NIL || right_type == LITERAL_NIL)
                err_msg = "Cant do addition with nil value";
            else if ((left_type == LITERAL_STRING || right_type == LITERAL_STRING) && left_type != right_type)
                err_msg = "Cant concat string with non-string value";
            // Determine type of whole expression
            else
                *final_type = ((left_type == LITERAL_STRING) ? LITERAL_STRING :
                              ((left_type == LITERAL_DOUBLE || right_type == LITERAL_DOUBLE) ? LITERAL_DOUBLE : LITERAL_INT));
            break;
        case T_DOUBLE_QUES:
        case T_EQUALS:
        case T_DIFFERS:
        case '=':
            if (left_type != right_type)
                err_msg = "Cant compare/assign different types";
            // Determine type of whole expression
            else
                *final_type = left_type;
            break;
        case '<':
        case '>':
        case T_LESS_OR_EQUAL:
        case T_GREATER_OR_EQUAL:
            if ((left_type != right_type) || (left_type == LITERAL_NIL || right_type == LITERAL_NIL))
                err_msg = "Cant compare different types/nil values";
            // Determine type of whole expression
            else
                *final_type = left_type;
            break;
        case '!':
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
    // check if ident is defined
    bool func_exists = get_type_from_table(function_decl->ident, context->func_type);
    if (!func_exists)
        return sema_err("Undefined function", ERR_UNDEF_FUNCTION);
    // check params - try to reuse function for funccall
    bool params_validity = check_func_params(function_decl->ident, function_decl->parameters);
    bool body_validity = check_block(function_decl->body);
    return params_validity && body_validity;
}

/////////////////////////////////////////////////////////////////////////

static bool check_var_decl(AstVariableDecl *variable_decl) {
    // check ident is valid
    AstType var_type;
    bool var_exists = check_variable(variable_decl->ident, &var_type);
    // check expr
    AstType expr_type;
    bool check_expr = process_expr(variable_decl->value, &expr_type);

    //TODO check if expr type is zjistitelny nebo ne
    if (expr_type == UNKNOWN)
        return sema_err("Expression type cant be determined", ERR_UNKNOWN_TYPE);
    //TODO check if data type is given, or determine it
    if (var_type == NOT_DEFINED)
        variable_decl->type = expr_type;
    else if (var_type != expr_type)
        return sema_err("Data types are not compatible", ERR_INCOM_TYPE);
}

/////////////////////////////////////////////////////////////////////////

static bool check_return(AstReturn *return_v, Context *context) {
    // maybe use global variable to for current context or pass it as another param
    AstType type;
    // Void function with return expression is wrong
    if (context->func_type == VOID_FUNC && !return_v->expr)
        return sema_err("Void funtion returns something", ERR_INVALID_RETURN);
    // Non-void function missing return expression
    if (context->func_type != VOID_FUNC && !return_v->expr)
        return sema_err("Missing function's return type", ERR_INVALID_RETURN);

    bool valid_expr = process_expr(return_v->expr, &type);
    // Different return types
    if (type != context->func_type)
        return sema_err("Function returns different type than expected", ERR_INVALID_FUN);
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

static AstType get_type_from_table(SymItem *ident, DataStruct *final_type) {
    // TODO FOR MARTIN, kdyz void funcke, aby to bylo ve finaltype rozpoznatelne
    return 0;
}

static Vec *get_args_vector_for_function(SymItem *funcname) {
    // TODO FOR MARTIN
    return NULL;
}

/////////////////////////////////////////////////////////////////////////
