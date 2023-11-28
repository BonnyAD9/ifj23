#include "semantics.h"

/////////////////////////////////////////////////////////////////////////
static bool compat_array[6][8][8] = {
    // *********************** '*', '-', '/' ***********************
    // INT   NOT_NIL  DOUBLE NOT_NIL  STRING NOT_NIL   NIL   NOT_NIL
   {{   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // INT
    {   0   ,   1   ,   0   ,   1   ,   0   ,   0   ,   0   ,   0   }, // INT_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // DOUBLE
    {   0   ,   1   ,   0   ,   1   ,   0   ,   0   ,   0   ,   0   }, // DOUBLE_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // STRING
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // STRING_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }},// NIL_NOT_NIL

    // **************************** '+' ****************************
    // INT   NOT_NIL  DOUBLE NOT_NIL  STRING NOT_NIL   NIL   NOT_NIL
   {{   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // INT
    {   0   ,   1   ,   0   ,   1   ,   0   ,   0   ,   0   ,   0   }, // INT_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // DOUBLE
    {   0   ,   1   ,   0   ,   1   ,   0   ,   0   ,   0   ,   0   }, // DOUBLE_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // STRING
    {   0   ,   0   ,   0   ,   0   ,   0   ,   1   ,   0   ,   0   }, // STRING_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }},// NIL_NOT_NIL

    // ******************* '==', '!=', '<=', '=>' ******************
    // INT   NOT_NIL  DOUBLE NOT_NIL  STRING NOT_NIL   NIL   NOT_NIL
   {{   1   ,   0   ,   0   ,   0   ,   0   ,   0   ,   1   ,   0   }, // INT
    {   0   ,   1   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // INT_NOT_NIL
    {   0   ,   0   ,   1   ,   0   ,   0   ,   0   ,   1   ,   0   }, // DOUBLE
    {   0   ,   1   ,   0   ,   1   ,   0   ,   0   ,   0   ,   0   }, // DOUBLE_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   1   ,   0   ,   1   ,   0   }, // STRING
    {   0   ,   0   ,   0   ,   0   ,   0   ,   1   ,   0   ,   0   }, // STRING_NOT_NIL
    {   1   ,   0   ,   1   ,   0   ,   1   ,   0   ,   1   ,   0   }, // NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   1   }},// NIL_NOT_NIL

    // *************************** '??' ****************************
    // INT   NOT_NIL  DOUBLE NOT_NIL  STRING NOT_NIL   NIL   NOT_NIL   [RIGHT / LEFT]
   {{   0   ,   1   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // INT
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // INT_NOT_NIL
    {   0   ,   0   ,   0   ,   1   ,   0   ,   0   ,   0   ,   0   }, // DOUBLE
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // DOUBLE_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   1   ,   0   ,   0   }, // STRING
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // STRING_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   1   }, // NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }},// NIL_NOT_NIL

    // **************************** '=' ****************************
    // INT   NOT_NIL  DOUBLE NOT_NIL  STRING NOT_NIL   NIL   NOT_NIL   [RIGHT / LEFT]
   {{   1   ,   1   ,   0   ,   0   ,   0   ,   0   ,   1   ,   1   }, // INT
    {   0   ,   1   ,   0   ,   0   ,   0   ,   0   ,   0   ,   1   }, // INT_NOT_NIL
    {   0   ,   0   ,   1   ,   1   ,   0   ,   0   ,   1   ,   1   }, // DOUBLE
    {   0   ,   0   ,   0   ,   1   ,   0   ,   0   ,   0   ,   1   }, // DOUBLE_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   1   ,   1   ,   1   ,   1   }, // STRING
    {   0   ,   0   ,   0   ,   0   ,   0   ,   1   ,   0   ,   1   }, // STRING_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }},// NIL_NOT_NIL

    // ************************* '<', '>' **************************
    // INT   NOT_NIL  DOUBLE NOT_NIL  STRING NOT_NIL   NIL   NOT_NIL
   {{   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // INT
    {   0   ,   1   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // INT_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // DOUBLE
    {   0   ,   0   ,   0   ,   1   ,   0   ,   0   ,   0   ,   0   }, // DOUBLE_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // STRING
    {   0   ,   0   ,   0   ,   0   ,   0   ,   1   ,   0   ,   0   }, // STRING_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }} // NIL_NOT_NIL
};

/*
   UNKNOWN.........0...DT_NONE
   INT.............3...DT_INT_NIL
   INT_NOT_NIL.....2...DT_INT
   DOUBLE..........5...DT_DOUBLE_NIL
   DOUBLE_NOT_NIL..4...DT_DOUBLE
   STRING..........9...DT_STRING_NIL
   STRING_NOT_NIL..8...DT_STRING
   NIL.............15..DT_ANY_NIL
   NIL_NOT_NIL.....14..DT_ANY
*/

static int final_type_arr[8][8] = {
    // INT   NOT_NIL  DOUBLE NOT_NIL  STRING NOT_NIL   NIL   NOT_NIL
    {   3   ,   3   ,   5   ,   5   ,   0   ,   0   ,   3   ,   3   }, // INT
    {   3   ,   2   ,   5   ,   4   ,   0   ,   0   ,   3   ,   2   }, // INT_NOT_NIL
    {   5   ,   5   ,   5   ,   5   ,   0   ,   0   ,   5   ,   5   }, // DOUBLE
    {   5   ,   4   ,   5   ,   4   ,   0   ,   0   ,   5   ,   4   }, // DOUBLE_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   9   ,   9   ,   4   ,   9   }, // STRING
    {   0   ,   0   ,   0   ,   0   ,   9   ,   8   ,   9   ,   8   }, // STRING_NOT_NIL
    {   3   ,   3   ,   5   ,   5   ,   4   ,   9   ,   15  ,   0   }, // NIL
    {   3   ,   2   ,   5   ,   4   ,   9   ,   8   ,   0   ,   14  }  // NIL_NOT_NIL
};

/////////////////////////////////////////////////////////////////////////
static bool sema_err(const char *msg, const int err_type);

static bool process_expr(AstExpr *expr);

static bool sem_process_binary(AstBinaryOp *binary_op);
static bool sem_process_unary(AstUnaryOp *unary_op);
static bool sem_process_call(AstFunctionCall *func_call);
static bool check_func_params(SymItem *ident, Vec args);
static bool sem_process_literal(AstLiteral *literal);
static bool sem_process_variable(SymItem *ident);

static unsigned int get_arr_index(DataType type);
static void check_in_array(unsigned int arr_sel, const char **err_msg, const char *err_msg_cnt, AstBinaryOp *binary_op);
static bool check_compatibility(AstBinaryOp *binary_op);

static bool sem_process_block(Vec stmts, bool top_level);
static bool sem_process_stmt(AstStmt *stmt, bool top_level);
bool sem_process_func_decl(AstFunctionDecl *func_decl);
static bool sem_process_var_decl(AstVariableDecl *variable_decl);
static bool sem_process_return(AstReturn *return_v);
static bool sem_process_expr_condition(AstExpr *expr);
static bool sem_process_let_condition(SymItem *ident);
static bool sem_process_if(AstIf *if_v);
static bool sem_process_while(AstWhile *while_v);
/////////////////////////////////////////////////////////////////////////

static bool sema_err(const char *msg, const int err_type) {
    set_err_code(err_type);
    EPRINTF("%s", msg);
    return false;
}

/////////////////////////////////////////////////////////////////////////

AstExpr *sem_expr(AstExpr *expr) {
    if (process_expr(expr))
        return expr;

    ast_free_expr(&expr);
    return NULL;
}

static bool process_expr(AstExpr *expr) {
    if (expr->sema_checked)
        return true;
    expr->sema_checked = true;

    bool ret_val;
    switch (expr->type) {
        case AST_BINARY_OP:
            ret_val = sem_process_binary(expr->binary_op);
            expr->data_type = expr->binary_op->data_type;
            return ret_val;
        case AST_UNARY_OP:
            ret_val = sem_process_unary(expr->unary_op);
            expr->data_type = expr->unary_op->data_type;
            return ret_val;
        case AST_FUNCTION_CALL:
            ret_val = sem_process_call(expr->function_call);
            expr->data_type = expr->function_call->data_type;
            return ret_val;
        case AST_LITERAL:
            ret_val = sem_process_literal(expr->literal);
            expr->data_type = expr->literal->data_type;
            return ret_val;
        case AST_VARIABLE:
            ret_val = sem_process_variable(expr->variable->ident);
            expr->data_type = expr->variable->ident->var.data_type;
            return ret_val;
        default:
            return sema_err("Uknown expression type", ERR_SEMANTIC);
    }
}

/////////////////////////////////////////////////////////////////////////
AstExpr *sem_binary(AstExpr *left, Token op, AstExpr *right) {
    AstExpr *binary_expr = ast_binary_op_expr(ast_binary_op(op, left, right));

    if (process_expr(binary_expr))
        return binary_expr;

    ast_free_expr(&binary_expr);
    return NULL;
}

static bool sem_process_binary(AstBinaryOp *binary_op) {
    if (binary_op->sema_checked)
        return true;

    // Avoid f.e. funccall(a,b) = 5 type of expressions
    AstExprType left_expr_type = binary_op->left->type;
    if ((left_expr_type == AST_FUNCTION_CALL || left_expr_type == AST_LITERAL) && binary_op->operator == '=')
        return sema_err("Invalid l-value of expression", ERR_SEMANTIC);

    bool left_side = process_expr(binary_op->left);
    bool right_side = process_expr(binary_op->right);

    if (!left_side || !right_side)
        // Error is already set by lower instancies
        return false;

    if (check_compatibility(binary_op))
        return true;
    return false;
}

/////////////////////////////////////////////////////////////////////////

AstExpr *sem_unary(AstExpr *expr, Token op) {
    AstExpr *unary_expr = ast_unary_op_expr(ast_unary_op(op, expr));

    if (process_expr(unary_expr))
        return unary_expr;

    ast_free_expr(&unary_expr);
    return NULL;
}

static bool sem_process_unary(AstUnaryOp *unary_op) {
    if (unary_op->sema_checked)
        return true;

    bool eval_expr = process_expr(unary_op->param);
    // Check for non-translateable options
    if (unary_op->param->data_type == DT_NONE || !eval_expr)
        return sema_err("Invalid expression, cant convert to non-nil type", ERR_SEMANTIC);

    // Only allowed unary operator is '!'
    if (unary_op->operator != '!')
        return sema_err("Invalid unary operator", ERR_SEMANTIC);

    // Switch to NOT_NIL values if not already set
    if (unary_op->param->data_type & DT_NIL)
        // Clear nillable flag - XOR
        unary_op->data_type = unary_op->param->data_type ^ DT_NIL;
    unary_op->sema_checked = true;

    return true;
}

/////////////////////////////////////////////////////////////////////////

SymItem *calle_ident(AstExpr *expr) {
    if (expr->type == AST_VARIABLE)
        return expr->variable->ident;
    if (expr->type == AST_UNARY_OP)
        return calle_ident(expr->unary_op->param);
    // Else error
    sema_err("Unknown function name", ERR_UNDEF_FUNCTION);
    return NULL;
}

AstExpr *sem_call(AstExpr *calle, Vec params) {
    SymItem* ident = calle_ident(calle);
    if (!ident)
        return NULL;

    AstExpr *func_call_expr = ast_function_call_expr(ast_function_call(ident, params));

    if (process_expr(func_call_expr))
        return func_call_expr;

    ast_free_expr(&func_call_expr);
    return NULL;
}

static bool sem_process_call(AstFunctionCall *func_call) {
    if (func_call->sema_checked)
        return true;

    if (!func_call->ident->declared)
        return sema_err("Undefined function", ERR_UNDEF_FUNCTION);

    func_call->data_type = sym_func_get_ret(func_call->ident, func_call->ident->name);

    if (check_func_params(func_call->ident, func_call->arguments)) {
        func_call->sema_checked = true;
        return true;
    }
    return false;
}

static bool check_func_params(SymItem *ident, Vec args) {
    Vec *params = sym_func_get_params(ident, ident->name);
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
        FuncParam param = VEC_AT(params, FuncParam, arg.i);
        DataType param_data_type, arg_data_type;

        // Load also param data type
        if (!sem_process_variable(param.ident))
            return false;
        param_data_type = param.ident->var.data_type;

        if (arg.v->type == AST_VARIABLE) {
            // Check if variable is defined
            if (!sem_process_variable(arg.v->variable))
                // Error already set by sem_process_variable()
                return false;
            arg_data_type = arg.v->variable->var.data_type;
        }
        else if (arg.v->type == AST_LITERAL) {
            if (!sem_process_literal(&(arg.v->literal)))
                // Error already set by sem_process_literal()
                return false;
            arg_data_type = arg.v->literal.data_type;
        }
        else
            return sema_err("Unexpected argument type", ERR_INVALID_FUN);

        // Check for correct ident name
        if (strcmp(arg.v->name.str, param.label.str))
            return sema_err("Expected func argument differs in names", ERR_INVALID_FUN);

        // Check if correct type
        if (param_data_type != arg_data_type) {
            if ((param_data_type & DT_NIL) && arg_data_type != DT_ANY_NIL) // DT_INT_NIL, DT_DOUBLE_NIL, DT_STRING_NIL, DT_ANY_NIL (NIL)
                return sema_err("Uncompatible argument and parameter type", ERR_INVALID_FUN);
            if (!(param_data_type & DT_NIL) && arg_data_type != DT_ANY) // DT_INT, DT_DOUBLE, DT_STRING, DT_NIL (NIL!)
                return sema_err("Uncompatible argument and parameter type", ERR_INVALID_FUN);
        }
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////

AstExpr *sem_literal(FullToken token) {
    AstLiteral *literal;

    if (token.subtype == DT_INT)
        literal = ast_int_literal(token.int_v);
    else if (token.subtype == DT_DOUBLE)
        literal = ast_double_literal(token.double_v);
    else if (token.subtype == DT_STRING)
        literal = ast_string_literal(token.str);
    else // DT_NIL
        literal = ast_nil_literal();

    AstExpr *lite_expr = ast_literal_expr(literal);

    if (process_expr(lite_expr))
        return lite_expr;

    ast_free_expr(&lite_expr);
    return NULL;
}

static bool sem_process_literal(AstLiteral *literal) {
    if (literal->sema_checked)
        return true;

    // Implicit conversion to DOUBLE type for INT literal
    if (literal->data_type == DT_INT || literal->data_type == DT_INT_NIL)
        // INT(2) -> DOUBLE(4) , INT_NIL(3) -> DOUBLE_NIL(5)
        literal->data_type += 2;
    literal->sema_checked = true;

    return true;
}

/////////////////////////////////////////////////////////////////////////

AstExpr *sem_variable(SymItem *ident) {
    AstExpr *var_expr = ast_variable_expr(ast_variable(ident));

    if (process_expr(var_expr))
        return var_expr;

    ast_free_expr(&var_expr);
    return NULL;
}

static bool sem_process_variable(SymItem *ident) {
    if (!ident->declared)
        return sema_err("Undeclared variable", ERR_UNDEF_FUNCTION);

    if (ident->type != SYM_VAR)
        return sema_err("Invalid type of ident provided", ERR_UNDEF_FUNCTION);

    if (ident->var.data_type & DT_NIL)
        // Throw warning about potencial nil
        WPRINTF("Variable can contain nil value");
    return true;
}

/////////////////////////////////////////////////////////////////////////

static unsigned int get_arr_index(DataType type) {
    if (type == DT_INT)        return 1;
    if (type == DT_DOUBLE)     return 3;
    if (type == DT_STRING)     return 5;

    if (type == DT_INT_NIL)    return 0;
    if (type == DT_DOUBLE_NIL) return 2;
    if (type == DT_STRING_NIL) return 4;

    if (type == DT_ANY)        return 7;
    if (type == DT_ANY_NIL ||
        type == DT_VOID)       return 6;
    // Unexpected state
    return 8;
}

static void check_in_array(unsigned int arr_sel, const char **err_msg, const char *err_msg_cnt, AstBinaryOp *binary_op) {
    DataType left_type  = binary_op->left->data_type,
             right_type = binary_op->right->data_type;

    AstExprType left_expr_type  = binary_op->left->type,
                right_expr_type = binary_op->right->type;

    // Check for unexpected type provided
    if (get_arr_index(left_type) == 8 || get_arr_index(right_type) == 8)
        *err_msg = "Unexpected type provided";
    else if (!compat_array[arr_sel][get_arr_index(left_type)][get_arr_index(right_type)]) {
        // Current check failed, if any of operands is LITERAL, try it again as these LITERALS are implicitly casted to DOUBLE
        // Motivation: f.e. if (a[Int] == 5[Int]) -> after implicit casting -> if (a[Int] == 5[Double]) => ERROR even when correct at the beggining

        if (left_expr_type == AST_LITERAL && (left_type == DT_DOUBLE || left_type == DT_DOUBLE_NIL)) {
            // Left operand is LITERAL - change type from DOUBLE -> INT and check again
            binary_op->left->data_type -= 2;
            check_in_array(arr_sel, err_msg, err_msg_cnt, binary_op);
            // Switch as need to respect implicit to double conversion
            binary_op->left->data_type += 2;
            return;
        }
        if (right_expr_type == AST_LITERAL && (right_type == DT_DOUBLE || right_type == DT_DOUBLE_NIL)) {
            // Right operand is LITERAL - change type from DOUBLE -> INT and check again
            binary_op->right->data_type -= 2;
            check_in_array(arr_sel, err_msg, err_msg_cnt, binary_op);
            // Switch as need to respect implicit to double conversion
            binary_op->right->data_type += 2;
            return;
        }
        // Set error message to non-NILL otherwise keep it NULL
        *err_msg = err_msg_cnt;
    }
    else
        binary_op->data_type = final_type_arr[get_arr_index(left_type)][get_arr_index(right_type)];
}

static bool check_compatibility(AstBinaryOp *binary_op) {
    if (binary_op->sema_checked)
        return true;

    const char *err_msg = NULL;
    switch (binary_op->operator) {
        case '*':
            check_in_array(0, &err_msg, "Uncompatible types for performing '*' operation", binary_op);
            break;
        case '-':
            check_in_array(0, &err_msg, "Uncompatible types for performing '-' operation", binary_op);
            break;
        case '/':
            check_in_array(0, &err_msg, "Uncompatible types for performing '/' operation", binary_op);
            break;
        ////////////////////
        case '+':
            check_in_array(1, &err_msg, "Uncompatible types for performing '+' operation", binary_op);
            break;
        ////////////////////
        case T_EQUALS:           // '=='
            check_in_array(2, &err_msg, "Uncompatible types for performing '==' operation", binary_op);
            break;
        case T_DIFFERS:          // '!='
            check_in_array(2, &err_msg, "Uncompatible types for performing '!=' operation", binary_op);
            break;
        case T_LESS_OR_EQUAL:    // '<='
            check_in_array(2, &err_msg, "Uncompatible types for performing '<=' operation", binary_op);
            break;
        case T_GREATER_OR_EQUAL: // '>='
            check_in_array(2, &err_msg, "Uncompatible types for performing '>=' operation", binary_op);
            break;
        ////////////////////
        case T_DOUBLE_QUES:  // ??
            check_in_array(3, &err_msg, "Uncompatible types for performing '\?\?' operation", binary_op);
            break;
        ////////////////////
        case '=':
            check_in_array(4, &err_msg, "Uncompatible types for performing '=' operation", binary_op);
            break;
        ////////////////////
        case '<':
            check_in_array(5, &err_msg, "Uncompatible types for performing '<' operation", binary_op);
            break;
        case '>':
            check_in_array(5, &err_msg, "Uncompatible types for performing '>' operation", binary_op);
            break;
        default:
            err_msg = "Uknown operator type";
            break;
    }

    // If set, output error along with error message
    if (err_msg)
        return sema_err(err_msg, ERR_INCOM_TYPE);
    binary_op->sema_checked = true;

    return true;
}

/////////////////////////////////////////////////////////////////////////

AstBlock *sem_block(Vec stmts, bool top_level) {
    AstBlock *block = ast_block(stmts);

    if (sem_process_block(stmts, top_level))
        return block;

    ast_free_block(&block);
    return NULL;
}

static bool sem_process_block(Vec stmts, bool top_level) {
    VEC_FOR_EACH(&(stmts), AstStmt*, stmt) {
        bool stmt_valid = sem_process_stmt((*stmt.v), top_level);
        if (!stmt_valid)
            // Error already set in sem_process_stmt()
            return false;
    }
    return true;
}

static bool sem_process_stmt(AstStmt *stmt, bool top_level) {
    if (stmt->sema_checked)
        return true;
    stmt->sema_checked = true;

    bool ret_val;
    switch (stmt->type) {
        case AST_EXPR:
            ret_val = process_expr(stmt->expr);
            stmt->data_type = stmt->expr->data_type;
            return ret_val;
        case AST_BLOCK:
            return sem_process_block(stmt->block->stmts, top_level);
        case AST_FUNCTION_DECL:
            if (!top_level)
                return sema_err("Cant declare function in function", ERR_SEMANTIC);
            return sem_process_func_decl(stmt->function_decl);
        case AST_VARIABLE_DECL:
            return sem_process_var_decl(stmt->variable_decl);
        case AST_RETURN:
            if (top_level)
                return sema_err("Return statement in main body is prohibited", ERR_SEMANTIC);
            ret_val = sem_process_return(stmt->return_v);
            stmt->data_type = stmt->return_v->data_type;
            return ret_val;
        case AST_IF:
            return sem_process_if(stmt->if_v);
        case AST_WHILE:
            return sem_process_while(stmt->while_v);
        default:
            return sema_err("Uknown stament type", ERR_SEMANTIC);
    }
}

/////////////////////////////////////////////////////////////////////////

AstStmt *sem_func_decl(SymItem *ident, Vec params, DataType return_type, AstBlock *body) {
    AstStmt *func_decl = ast_function_decl_stmt(ast_function_decl(ident, params, body));

    if (sem_process_stmt(func_decl, true))
        return func_decl;

    ast_free_stmt(&func_decl);
    return NULL;
}

bool sem_process_func_decl(AstFunctionDecl *func_decl) {
    if (func_decl->sema_checked)
        return true;

    if (!func_decl->ident->declared)
        return sema_err("Undeclared function", ERR_UNDEF_FUNCTION);

    if (!check_func_params(func_decl->ident, func_decl->parameters))
        // Error already set in check_func_params()
        return false;

    // First check body is valid
    bool func_body = sem_process_block(func_decl->body->stmts, false);
    if (!func_body)
        // Error already set in sem_block()
        return false;

    // Now iterate through valid body for return statement
    VEC_FOR_EACH(&(func_decl->body->stmts), AstStmt*, stmt) {
        if ((*stmt.v)->type == AST_RETURN) {
            DataType return_type = sym_func_get_ret(func_decl->ident, func_decl->ident->name);
            if (return_type == DT_VOID && (*stmt.v)->return_v->expr)
                return sema_err("Unexpected return expression for void type function", ERR_INVALID_RETURN);

            if (return_type != DT_VOID && !(*stmt.v)->return_v->expr)
                return sema_err("Missing return expression for function", ERR_INVALID_RETURN);

            process_expr((*stmt.v)->return_v->expr);
            // Check for type compatibility (same as for '=')
            if (!compat_array[4][get_arr_index(return_type)][get_arr_index((*stmt.v)->return_v->expr->data_type)])
                return sema_err("Uncompatible types for function return", ERR_INVALID_FUN);
        }
    }
    func_decl->sema_checked = true;

    return true;
}

/////////////////////////////////////////////////////////////////////////
AstStmt *sem_var_decl(bool mutable, SymItem *ident, DataType type, AstExpr *expr) {
    if (!sem_process_variable(ident))
        // Error already set in sem_process_variable()
        return NULL;

    ident->var.data_type = type;
    ident->var.mutable = mutable;
    AstStmt *var_decl = ast_variable_decl_stmt(ast_variable_decl(ident, expr));

    if (sem_process_stmt(var_decl, true)) // top_level is not relevant here
        return var_decl;

    ast_free_stmt(&var_decl);
    return NULL;
}

static bool sem_process_var_decl(AstVariableDecl *variable_decl) {
    if (variable_decl->sema_checked)
        return true;

    // Check expression
    if (variable_decl->value) {
        bool check_expr = process_expr(variable_decl->value);

        // Check if expr type is inferiable
        if (!check_expr)
            return sema_err("Invalid expression", ERR_UNKNOWN_TYPE);

        // Check if data type is given, or try determine it
        if (variable_decl->ident->var.data_type == DT_NONE) {
            if (variable_decl->value->data_type == DT_ANY_NIL)
                return sema_err("Expression type cant be determined", ERR_UNKNOWN_TYPE);
            variable_decl->ident->var.data_type = variable_decl->value->data_type;
        }
        else
            return check_compatibility(&((AstBinaryOp) {
                .operator = '=',
                .left = &((AstExpr) {
                    .data_type = variable_decl->ident->var.data_type,
                    .type      = AST_VARIABLE
                }),
                .right = &((AstExpr) {
                    .data_type = variable_decl->value->data_type,
                    .type      = variable_decl->value->type
                })
            }));
    }
    else if (variable_decl->ident->var.data_type == DT_NONE)
        // Case when type and expression are missing
        return sema_err("Variable declaration is missing both type and expression", ERR_UNKNOWN_TYPE);
    variable_decl->sema_checked = true;
    // Else nothing to do, type is given but without expression
    return true;
}

/////////////////////////////////////////////////////////////////////////

AstStmt *sem_return(AstExpr *expr) {
    AstStmt *ret_stmt = ast_return_stmt(ast_return(expr));

    if (sem_process_stmt(ret_stmt, false))
        return ret_stmt;

    ast_free_stmt(&ret_stmt);
    return NULL;
}

static bool sem_process_return(AstReturn *return_v) {
    if (return_v->sema_checked)
        return true;

    bool ret_expr = process_expr(return_v->expr);
    if (!ret_expr)
        // Error already set in sem_expr()
        return false;

    return_v->sema_checked = true;
    return_v->data_type = return_v->expr->data_type;

    return true;
}

/////////////////////////////////////////////////////////////////////////

AstCondition *sem_expr_condition(AstExpr *expr) {
    AstCondition *expr_cond = ast_expr_condition(expr);

    if (sem_process_expr_condition(expr_cond->expr)) {
        expr_cond->sema_checked = true;
        return expr_cond;
    }

    ast_free_condition(&expr_cond);
    return NULL;
}

static bool sem_process_expr_condition(AstExpr *expr) {
    if (expr->sema_checked)
        return true;

    // Check if expression is correct
    bool cond_expr = process_expr(expr);
    if (!cond_expr)
        // Error already set in sem_expr()
        return false;
    // Check if given expression becomes only TRUE/FALSE
    if (expr->type != AST_BINARY_OP)
        return sema_err("Statement condition cant be evaluated", ERR_INCOM_TYPE);
    else {
        // Check for relational operators usage (f.e. operator '+' doesnt make sense and cant be evaluated as true/false)
        Token oper = expr->binary_op->operator;
        // T_EQUALS, T_DIFFERS, T_LESS_OR_EQUAL, T_GREATER_OR_EQUAL, '>', '<'
        if ((oper < T_EQUALS || oper > T_GREATER_OR_EQUAL) && oper != '<' && oper != '>')
            return sema_err("Statement condition cant be evaluated", ERR_INCOM_TYPE);
    }
    expr->sema_checked = true;

    return true;
}

/////////////////////////////////////////////////////////////////////////

AstCondition *sem_let_condition(SymItem *ident) {
    AstCondition *let_cond = ast_let_condition(ident);

    if (sem_process_let_condition(let_cond->let)) {
        let_cond->sema_checked = true;
        return let_cond;
    }

    ast_free_condition(&let_cond);
    return NULL;
}

static bool sem_process_let_condition(SymItem *ident) {
    if (!ident->declared)
        return sema_err("Undeclared variable", ERR_UNDEF_VAR);

    if (ident->type != SYM_VAR)
        return sema_err("Invalid type of ident provided", ERR_SEMANTIC);

    if (ident->var.mutable)
        return sema_err("Mutable variable provided", ERR_SEMANTIC);
    return true;
}

/////////////////////////////////////////////////////////////////////////

AstStmt *sem_if(AstCondition *cond, AstBlock *true_block, AstBlock *false_block) {
    AstStmt *if_stmt = ast_if_stmt(ast_if(cond, true_block, false_block));

    if (sem_process_stmt(if_stmt, false)) // top_level is not relevant here
        return if_stmt;

    ast_free_stmt(&if_stmt);
    return NULL;
}

static bool sem_process_if(AstIf *if_v) {
    if (if_v->sema_checked)
        return true;

    bool if_cond;
    if (if_v->condition->type == AST_COND_EXPR)
        if_cond = sem_process_expr_condition(if_v->condition->expr);
    else // AST_COND_LET
        if_cond = sem_process_let_condition(if_v->condition->let);

    if (!if_cond)
        // Error already set in above func
        return false;

    if (!if_v->if_body)
        return sema_err("Missing statement body", ERR_SEMANTIC);
    // Process if (and) else body
    bool if_body = sem_process_block(if_v->if_body->stmts, false);
    if (!if_body)
        // Error already set in sem_block()
        return false;
    if (if_v->else_body) {
        bool else_body = sem_process_block(if_v->else_body->stmts, false);
        if (!else_body)
            // Error already set in sem_block()
            return false;
    }
    if_v->sema_checked = true;

    return true;
}

/////////////////////////////////////////////////////////////////////////

AstStmt *sem_expr_stmt(AstExpr *expr) {
    if (sem_expr(expr))
        return ast_expr_stmt(expr);
    return NULL;
}

/////////////////////////////////////////////////////////////////////////

AstStmt *sem_while(AstCondition *cond, AstBlock *loop) {
    AstStmt *while_stmt = ast_while_stmt(ast_while(cond, loop));

    if (sem_process_stmt(while_stmt, false)) // top_level is not relevant here
        return while_stmt;

    ast_free_stmt(&while_stmt);
    return NULL;
}

static bool sem_process_while(AstWhile *while_v) {
    if (while_v->sema_checked)
        return true;

    bool while_cond;
    if (while_v->condition->type == AST_COND_EXPR)
        while_cond = sem_process_expr_condition(while_v->condition->expr);
    else // AST_COND_LET
        while_cond = sem_process_let_condition(while_v->condition->let);

    if (!while_cond)
        // Error already set in above func
        return false;

    if (!while_v->body)
        return sema_err("Missing while() body", ERR_SEMANTIC);
    // Process cycle body
    bool while_body = sem_process_block(while_v->body->stmts, false);
    if (!while_body)
        // Error already set in sem_block()
        return false;
    while_v->sema_checked = true;

    return true;
}

/////////////////////////////////////////////////////////////////////////

bool sem_func_param(String label, SymItem *ident, DataType type, FuncParam *res) {
    // Check for correct param type (not VOID, NIL ...)
    if (type != DT_VOID && type != DT_ANY && type != DT_ANY_NIL && type != DT_NONE) {
        *res = sym_func_param_new(ident, label);
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////

bool sem_lex_literal(Lexer *lex, AstLiteral *res) {
    if (res->sema_checked)
        return true;

    if (lex->cur != T_LITERAL)
        return false;

    if (lex->subtype == DT_INT)
        res = ast_int_literal(lex->i_num);
    else if (lex->subtype == DT_DOUBLE)
        res = ast_double_literal(lex->d_num);
    else if (lex->subtype == DT_STRING)
        res = ast_string_literal(lex->str);
    else // DT_NIL
        res = ast_nil_literal();
    res->sema_checked = true;

    return true;
}
