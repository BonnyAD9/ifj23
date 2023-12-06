#include "semantics.h"
#include "debug_tools.h"

/////////////////////////////////////////////////////////////////////////
// Global variable for storing parent file position of variable
FilePos var_pos = (FilePos) {};
// Global variable for telling function check whether on top level or not
bool sem_top_level = false;
Context context = {
    .in_func        = false,
    .ret_stmt_found = false,
    .in_func_call   = false,
    .func_ret_type  = DT_NONE
};
/////////////////////////////////////////////////////////////////////////

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
static bool sema_err(FilePos pos, const char *msg, const int err_type);

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

static bool sem_process_block(Vec stmts);
static bool sem_process_stmt(AstStmt *stmt);
bool sem_process_func_decl(AstFunctionDecl *func_decl);
static bool sem_process_var_decl(AstVariableDecl *variable_decl);
static bool sem_process_return(AstReturn *return_v);
static bool sem_process_expr_condition(AstExpr *expr);
static bool sem_process_let_condition(SymItem *ident);
static bool sem_process_if(AstIf *if_v);
static bool sem_process_while(AstWhile *while_v);

static bool handle_write_func(SymItem *ident, Vec args);

// HELP FUNCS
static void set_block_checked(AstBlock* block, bool* sema_to_check);
static bool use_before_decl(FilePos decl, FilePos usage);
static bool not_another_func_decl(Vec stmts);
static void check_adapt_to_conversion(AstExpr *expr, DataType new_data_type);
bool check_func_decl_params_duplicity(Vec parameters);
/////////////////////////////////////////////////////////////////////////

static bool sema_err(FilePos pos, const char *msg, const int err_type) {
    //if (!err_code_set()) {
        set_err_code(err_type);
        EPRINTF(DEBUG_FILE ":%zu:%zu: error: %s\n", pos.line, pos.column, msg);
    //}
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

    bool ret_val;
    var_pos = expr->pos;

    switch (expr->type) {
        case AST_BINARY_OP:
            ret_val = sem_process_binary(expr->binary_op);
            expr->data_type = expr->binary_op->data_type;
            expr->sema_checked = expr->binary_op->sema_checked;
            return ret_val;
        case AST_UNARY_OP:
            ret_val = sem_process_unary(expr->unary_op);
            expr->data_type = expr->unary_op->data_type;
            expr->sema_checked = expr->unary_op->sema_checked;
            return ret_val;
        case AST_FUNCTION_CALL:
            ret_val = sem_process_call(expr->function_call);
            expr->data_type = expr->function_call->ident->func.return_type;
            expr->sema_checked = expr->function_call->sema_checked;
            return ret_val;
        case AST_LITERAL:
            ret_val = sem_process_literal(expr->literal);
            expr->data_type = expr->literal->data_type;
            expr->sema_checked = expr->literal->sema_checked;
            return ret_val;
        case AST_VARIABLE:
            ret_val = sem_process_variable(expr->variable->ident);
            expr->data_type = expr->variable->data_type = expr->variable->ident->var.data_type;
            expr->sema_checked = expr->variable->sema_checked = (expr->variable->ident->declared && expr->variable->ident->var.data_type != DT_NONE);
            return ret_val;
        default:
            return sema_err(expr->pos, "Uknown expression type", ERR_SEMANTIC);
    }
}

/////////////////////////////////////////////////////////////////////////

AstExpr *sem_binary(FilePos pos, AstExpr *left, Token op, AstExpr *right) {
    AstExpr *binary_expr = ast_binary_op_expr(
        left->pos,
        ast_binary_op(pos, op, left, right)
    );
    var_pos = pos;

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
    if ((left_expr_type == AST_FUNCTION_CALL || left_expr_type == AST_LITERAL)
        && binary_op->operator == '='
    ) {
        return sema_err(
            binary_op->left->pos,
            "Invalid l-value of expression",
            ERR_SEMANTIC
        );
    }

    if ((binary_op->left->data_type == DT_NONE || binary_op->right->data_type == DT_NONE) && !sem_top_level)
        return true;

    // Trying to work with function like with variable
    if ((binary_op->right->type == AST_VARIABLE && binary_op->right->variable->ident->type == SYM_FUNC) ||
        (binary_op->left->type == AST_VARIABLE && binary_op->left->variable->ident->type == SYM_FUNC)
    ) {
        return sema_err(
            binary_op->left->pos,
            "Cant handle function as variable",
            ERR_SEMANTIC
        );
    }

    if (binary_op->operator == '=') {
        // Trying to assign something to let variable outside of its own declaration
        if (binary_op->left->type == AST_VARIABLE && !binary_op->left->variable->ident->var.mutable && binary_op->left->variable->ident->declared) {
            return sema_err(
                binary_op->left->pos,
                "Cant change non-mutable variable's value",
                ERR_SEMANTIC
            );
        }
    }

    bool left_side = process_expr(binary_op->left);
    bool right_side = process_expr(binary_op->right);

    if (sem_top_level && binary_op->left->type == AST_VARIABLE && binary_op->left->data_type == DT_NONE)
        binary_op->left->data_type = binary_op->left->variable->ident->var.data_type;

    if (!left_side || !right_side)
        // Error is already set by lower instancies
        return false;

    // In case of any part of binary expr being unchecked, wait for top_level true to check
    if (!binary_op->left->sema_checked || !binary_op->right->sema_checked)
        return true;

    if (check_compatibility(binary_op)) {
        check_adapt_to_conversion(binary_op->right, binary_op->data_type);
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////

AstExpr *sem_unary(FilePos pos, AstExpr *expr, Token op) {
    AstExpr *unary_expr = ast_unary_op_expr(
        pos_min(pos, expr->pos),
        ast_unary_op(pos, op, expr)
    );
    var_pos = pos;

    if (process_expr(unary_expr))
        return unary_expr;

    ast_free_expr(&unary_expr);
    return NULL;
}

static bool sem_process_unary(AstUnaryOp *unary_op) {
    if (unary_op->sema_checked)
        return true;

    bool eval_expr = process_expr(unary_op->param);

    if (!unary_op->param->sema_checked)
        return true;

    // Check for non-translateable options
    if (unary_op->param->data_type == DT_NONE || !eval_expr) {
        return sema_err(
            unary_op->param->pos,
            "Invalid expression, cant convert to non-nil type",
            ERR_SEMANTIC
        );
    }

    unary_op->data_type = unary_op->param->data_type;
    // Switch to NOT_NIL values if not already set
    if (unary_op->param->data_type & DT_NIL)
        // Clear nillable flag - XOR
        unary_op->data_type = unary_op->param->data_type ^ DT_NIL;
    unary_op->sema_checked = unary_op->param->sema_checked;

    return true;
}

/////////////////////////////////////////////////////////////////////////

SymItem *calle_ident(AstExpr *expr) {
    if (expr->type == AST_VARIABLE)
        return expr->variable->ident;
    if (expr->type == AST_UNARY_OP)
        return calle_ident(expr->unary_op->param);
    // Else error
    sema_err(expr->pos, "Unknown function name", ERR_UNDEF_FUNCTION);
    return NULL;
}

AstExpr *sem_call(FilePos pos, AstExpr *calle, Vec params) {
    SymItem* ident = calle_ident(calle);
    ast_free_expr(&calle);
    var_pos = pos;

    if (!ident) {
        // Error already set in calle_ident()
        vec_free_with(&params, (FreeFun)ast_free_func_call_param);
        return NULL;
    }

    AstExpr *func_call_expr = ast_function_call_expr(
        ident->file_pos,
        ast_function_call(pos, ident, params)
    );

    if (process_expr(func_call_expr))
        return func_call_expr;

    ast_free_expr(&func_call_expr);
    return NULL;
}

static bool sem_process_call(AstFunctionCall *func_call) {
    if (func_call->sema_checked)
        return true;

    if (!func_call->ident->declared) {
        if (!sem_top_level)
            return true;

        return sema_err(
            func_call->pos,
            "Undefined function",
            ERR_UNDEF_FUNCTION
        );
    }

    func_call->data_type = sym_func_get_ret(func_call->ident, func_call->ident->name);

    if (check_func_params(func_call->ident, func_call->arguments)) {
        // bylo tady funccall-semachedk = top+level
        func_call->sema_checked = (func_call->ident->func.return_type != DT_NONE);
        return true;
    }
    return false;
}

static bool handle_write_func(SymItem *ident, Vec args) {
    VEC_FOR_EACH(&(args), AstFuncCallParam, arg) {
        // write() function cant have param's name, f.e. write(paramname : "ahoj", ...)
        if (arg.v->name.str != NULL && arg.v->name.len) {
            return sema_err(
                ident->file_pos,
                "Param's names arent allowed for write() function",
                ERR_INVALID_FUN
            );
        }

        if (arg.v->type == AST_VARIABLE) {
            var_pos = arg.v->variable->file_pos;
            context.in_func_call = true;

            // Check if variable is defined
            if (!sem_process_variable(arg.v->variable))
                // Error already set by sem_process_variable()
                return false;

            context.in_func_call = false;
        }
        else if (arg.v->type == AST_LITERAL) {
            var_pos = arg.v->literal.pos;
            if (!sem_process_literal(&(arg.v->literal)))
                // Error already set by sem_process_literal()
                return false;
        }
    }
    return true;
}

static bool check_func_params(SymItem *ident, Vec args) {
    // Special case for write() function
    if (!strcmp(ident->name.str, "write"))
        return handle_write_func(ident, args);

    Vec *params = sym_func_get_params(ident, ident->name);
    if (!params) {
        return sema_err(
            ident->file_pos,
            "Error while retrieving function params",
            ERR_SEMANTIC
        );
    }
    // Check for same vector length
    if (params->len != args.len) {
        return sema_err(
            ident->file_pos,
            "Provided count of args differs from func's count of params",
            ERR_SEMANTIC
        );
    }
    /*
        func decrement(of n: Int, by m: Int) -> Int { [params]
            return n - m
        }
        let decremented_n = decrement(of: n, by: 1)   [arguments]
    */
    FuncParam param;
    DataType param_data_type, arg_data_type;

    VEC_FOR_EACH(&(args), AstFuncCallParam, arg) {
        param = VEC_AT(params, FuncParam, arg.i);

        param_data_type = param.ident->var.data_type;
        if (arg.v->type == AST_VARIABLE) {
            var_pos = arg.v->variable->file_pos;
            context.in_func_call = true;

            // Check if variable is defined
            if (!sem_process_variable(arg.v->variable))
                // Error already set by sem_process_variable()
                return false;

            context.in_func_call = false;
            arg_data_type = arg.v->variable->var.data_type;
        }
        else if (arg.v->type == AST_LITERAL) {
            var_pos = arg.v->literal.pos;
            if (!sem_process_literal(&(arg.v->literal)))
                // Error already set by sem_process_literal()
                return false;
            arg_data_type = arg.v->literal.data_type;
        }
        else {
            return sema_err(
                ident->file_pos,
                "Unexpected argument type",
                ERR_INVALID_FUN
            );
        }

        // Check for correct ident name
        if (!str_eq(arg.v->name, param.label)) {
            return sema_err(
                var_pos,
                "Expected func argument differs in names",
                ERR_INVALID_FUN
            );
        }
        // Check if correct type
        if (param_data_type != arg_data_type) {
            // DT_INT_NIL, DT_DOUBLE_NIL, DT_STRING_NIL, DT_ANY_NIL (NIL)
            if ((param_data_type & DT_NIL) && arg_data_type != DT_ANY_NIL) {
                return sema_err(
                    var_pos,
                    "Uncompatible argument and parameter type",
                    ERR_INVALID_FUN
                );
            }
            // DT_INT, DT_DOUBLE, DT_STRING, DT_NIL (NIL!)
            if (!(param_data_type & DT_NIL) && arg_data_type != DT_ANY) {
                return sema_err(
                    var_pos,
                    "Uncompatible argument and parameter type",
                    ERR_INVALID_FUN
                );
            }
        }
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////

AstExpr *sem_literal(FilePos pos, FullToken token) {
    AstLiteral *literal;
    var_pos = pos;

    if (token.subtype == DT_INT)
        literal = ast_int_literal(pos, token.int_v);
    else if (token.subtype == DT_DOUBLE)
        literal = ast_double_literal(pos, token.double_v);
    else if (token.subtype == DT_STRING)
        literal = ast_string_literal(pos, token.str);
    else // DT_NIL
        literal = ast_nil_literal(pos);

    AstExpr *lite_expr = ast_literal_expr(pos, literal);

    if (process_expr(lite_expr))
        return lite_expr;

    ast_free_expr(&lite_expr);
    return NULL;
}

static bool sem_process_literal(AstLiteral *literal) {
    if (literal->sema_checked)
        return true;

    literal->sema_checked = true;
    return true;
}

/////////////////////////////////////////////////////////////////////////

static bool use_before_decl(FilePos decl, FilePos usage) {
    if (((usage.line) < (decl.line)) || (decl.column == decl.line && decl.line == 0))
        return true;
    return false;
}

AstExpr *sem_variable(FilePos pos, SymItem *ident) {
    AstExpr *var_expr = ast_variable_expr(
        pos,
        ast_variable(pos, ident)
    );
    var_pos = pos;

    if (!var_expr) {
        return NULL;
    }

    if (process_expr(var_expr))
        return var_expr;

    ast_free_expr(&var_expr);
    return NULL;
}

static bool sem_process_variable(SymItem *ident) {
    if (sem_top_level && use_before_decl(ident->file_pos, var_pos)) {
        return sema_err(
            var_pos,
            "Undeclared variable",
            ERR_UNDEF_FUNCTION
        );
    }

    if (context.in_func_call) {
        // Trying to work with function like with variable
        if (ident->type == SYM_FUNC) {
            return sema_err(
                var_pos,
                "Cant handle function as variable",
                ERR_SEMANTIC
            );
        }
    }

    if (!ident->declared && !sem_top_level)
        return true;

    if (ident->type != SYM_VAR && ident->type != SYM_FUNC) {
        return sema_err(
            var_pos,
            "Invalid type of ident provided",
            ERR_UNDEF_FUNCTION
        );
    }

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
        *err_msg = "Cant find data type - probably undefined";
    else if (!compat_array[arr_sel][get_arr_index(left_type)][get_arr_index(right_type)]) {
        // If an error occured, check for any literal involved and try it agin with to-DOUBLE casting
        if (left_expr_type == AST_LITERAL && (left_type == DT_INT || left_type == DT_INT_NIL)) {
            // Left operand is LITERAL - change type from INT -> DOUBLE and check again
            binary_op->left->data_type += 2;
            check_in_array(arr_sel, err_msg, err_msg_cnt, binary_op);
            return;
        }
        if (right_expr_type == AST_LITERAL && (right_type == DT_INT || right_type == DT_INT_NIL)) {
            // Right operand is LITERAL - change type from INT -> DOUBLE and check again
            binary_op->right->data_type += 2;
            check_in_array(arr_sel, err_msg, err_msg_cnt, binary_op);
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
    switch ((int)binary_op->operator) {
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

    binary_op->sema_checked = binary_op->left->sema_checked && binary_op->right->sema_checked;

    if (err_msg)
        // If set, output error along with error message
        return sema_err(binary_op->pos, err_msg, ERR_INCOM_TYPE);

    return true;
}

/////////////////////////////////////////////////////////////////////////

AstBlock *sem_block(FilePos pos, Vec stmts, bool top_level) {
    AstBlock *block = ast_block(pos, stmts);
    sem_top_level = top_level;

    bool ret_val = sem_process_block(stmts);
    // if top_level is true, after the cycle, everything should have sema_checked true, otherwise unexpected error
    if (top_level) {
        bool block_valid;
        set_block_checked(block, &block_valid);
        if (!block_valid)
            WPRINTF("Unexpected sema_checked value after top_level true block");
    }
    if (ret_val)
        return block;

    ast_free_block(&block);
    return NULL;
}

static bool sem_process_block(Vec stmts) {
    VEC_FOR_EACH(&(stmts), AstStmt*, stmt) {
        bool stmt_valid = sem_process_stmt((*stmt.v));
        if (!stmt_valid) {
            (*stmt.v)->sema_checked = true;
            // Error already set in sem_process_stmt()
            return false;
        }
    }
    return true;
}

static bool sem_process_stmt(AstStmt *stmt) {
    /*
        Return stmt will be checked and marked as sema_checked. However at the end, if present
        in main body, we have to check it for main body context.
    */
    if (stmt->sema_checked && stmt->type != AST_RETURN)
        return true;

    var_pos = stmt->pos;
    bool ret_val;

    switch (stmt->type) {
        case AST_EXPR:
            ret_val = process_expr(stmt->expr);
            stmt->data_type = stmt->expr->data_type;
            stmt->sema_checked = stmt->expr->sema_checked;

            return ret_val;
        case AST_BLOCK:
            return sem_process_block(stmt->block->stmts);
        case AST_FUNCTION_DECL:
            context.in_func = true;
            ret_val = sem_process_func_decl(stmt->function_decl);
            stmt->sema_checked = stmt->function_decl->sema_checked;
            context.in_func = context.ret_stmt_found = false; // (MAYBE WRONG)

            return ret_val;
        case AST_VARIABLE_DECL:
            ret_val = sem_process_var_decl(stmt->variable_decl);
            stmt->sema_checked = stmt->variable_decl->sema_checked;

            return ret_val;
        case AST_RETURN:
            if (!context.in_func && sem_top_level) {
                return sema_err(
                    stmt->pos,
                    "Return statement in main body is prohibited",
                    ERR_SEMANTIC
                );
            }

            ret_val = sem_process_return(stmt->return_v);
            stmt->data_type = stmt->return_v->data_type;
            stmt->sema_checked = stmt->return_v->sema_checked;

            return ret_val;
        case AST_IF:
            ret_val = sem_process_if(stmt->if_v);
            stmt->sema_checked = stmt->if_v->sema_checked;

            return ret_val;
        case AST_WHILE:
            ret_val = sem_process_while(stmt->while_v);
            stmt->sema_checked = stmt->while_v->sema_checked;

            return ret_val;
        default:
            return sema_err(stmt->pos, "Uknown stament type", ERR_SEMANTIC);
    }
}

/////////////////////////////////////////////////////////////////////////

bool check_func_decl_params_duplicity(Vec parameters) {
    VEC_FOR_EACH(&(parameters), FuncParam, func_param) {
        for (unsigned int index = (func_param.i + 1); index < parameters.len; ++index) {
            FuncParam param;
            param = VEC_AT(&parameters, FuncParam, index);
            // Check for labels
            if (func_param.v->label.str && str_eq(func_param.v->label, param.label)) {
                return sema_err(
                    var_pos,
                    "Duplicit names for function declaration labels",
                    ERR_SEMANTIC
                );
            }
            // Check for variable's names
            if (str_eq(func_param.v->ident->name, param.ident->name)) {
                return sema_err(
                    var_pos,
                    "Duplicit names for function declaration idents",
                    ERR_SEMANTIC
                );
            }
        }
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////

AstStmt *sem_func_decl(
    FilePos pos,
    SymItem *ident,
    AstBlock *body
) {
    AstStmt *func_decl = ast_function_decl_stmt(
        pos,
        ast_function_decl(pos, ident, ident->func.params, body)
    );
    var_pos = pos;

    if (sem_process_stmt(func_decl))
        return func_decl;

    ast_free_stmt(&func_decl);
    return NULL;
}

bool sem_process_func_decl(AstFunctionDecl *func_decl) {
    if (func_decl->sema_checked)
        return true;

    context.func_ret_type = sym_func_get_ret(func_decl->ident, func_decl->ident->name);

    // Check for valid params
    if (!check_func_decl_params_duplicity(func_decl->parameters))
        // Error already set in check_func_decl_params_duplicity()
        return false;

    // First check body is valid
    bool func_body = sem_process_block(func_decl->body->stmts);
    if (!func_body)
        // Error already set in sem_block()
        return false;
    // Check for another func_decl in this func's body
    if(!not_another_func_decl(func_decl->body->stmts))
        return false;

    if (sym_func_get_ret(func_decl->ident, func_decl->ident->name) != DT_VOID && !context.ret_stmt_found) {
        return sema_err(
            func_decl->pos,
            "Missing return statement for function",
            ERR_SYNTAX
        );
    }
    set_block_checked(func_decl->body, &(func_decl->sema_checked));

    return true;
}

/////////////////////////////////////////////////////////////////////////

static void check_adapt_to_conversion(AstExpr *expr, DataType new_data_type) {
    if (expr->type == AST_LITERAL &&
        expr->literal->data_type != new_data_type &&
        (new_data_type - 2) == expr->literal->data_type &&
       (expr->literal->data_type == DT_INT || expr->literal->data_type == DT_INT_NIL)
    ) {
        // Check if implicit conversion happened and if so update literal's datatype and value
        expr->literal->data_type = new_data_type;
        expr->literal->double_v = expr->literal->int_v;
        expr->data_type = new_data_type;
    }
}

/////////////////////////////////////////////////////////////////////////

AstStmt *sem_var_decl(FilePos pos, SymItem *ident, AstExpr *expr) {
    AstStmt *var_decl = ast_variable_decl_stmt(
        pos,
        ast_variable_decl(pos, ident, expr)
    );
    var_pos = pos;

    if (sem_process_stmt(var_decl))
        return var_decl;

    ast_free_stmt(&var_decl);
    return NULL;
}

static bool sem_process_var_decl(AstVariableDecl *variable_decl) {
    var_pos = variable_decl->pos;

    if (variable_decl->sema_checked)
        return true;

    if (!sem_process_variable(variable_decl->ident))
        // Error already set in sem_process_variable()
        return false;

    // Check expression
    if (variable_decl->value) {
        bool check_expr = process_expr(variable_decl->value);

        // Check if expr type is inferiable
        if (!check_expr) {
            return sema_err(
                variable_decl->value->pos,
                "Invalid expression",
                ERR_UNKNOWN_TYPE
            );
        }

        if (variable_decl->value->data_type == DT_NONE && !sem_top_level)
            return true;

        // Trying to work with function like with variable
        if (variable_decl->value->type == AST_VARIABLE && variable_decl->value->variable->ident->type == SYM_FUNC) {
            return sema_err(
                variable_decl->value->pos,
                "Cant handle function as variable",
                ERR_SEMANTIC
            );
        }

        variable_decl->sema_checked = variable_decl->value->sema_checked;
        // In case expression is notchecked yet - wait for top_level
        if (!variable_decl->sema_checked)
            return true;

        // Check if data type is given, or try determine it
        if (variable_decl->ident->var.data_type == DT_NONE) {
            if (variable_decl->value->data_type == DT_ANY_NIL || variable_decl->value->data_type == DT_NONE || variable_decl->value->data_type == DT_VOID) {
                return sema_err(
                    variable_decl->value->pos,
                    "Expression type can't be determined",
                    ERR_UNKNOWN_TYPE
                );
            }
            variable_decl->ident->var.data_type = variable_decl->value->data_type;
        }
        else {
            AstBinaryOp temp_binary_op = (AstBinaryOp) {
                .operator = '=',
                .left = &((AstExpr) {
                    .data_type    = variable_decl->ident->var.data_type,
                    .type         = AST_VARIABLE,
                    .sema_checked = true
                }),
                .right = &((AstExpr) {
                    .data_type    = variable_decl->value->data_type,
                    .type         = variable_decl->value->type,
                    .sema_checked = variable_decl->value->sema_checked
                })
            };

            bool ret_val = check_compatibility(&(temp_binary_op));
            variable_decl->sema_checked = temp_binary_op.sema_checked;
            check_adapt_to_conversion(variable_decl->value, temp_binary_op.data_type);

            return ret_val;
        }
    }
    else if (variable_decl->ident->var.data_type == DT_NONE) {
        // Case when type and expression are missing
        return sema_err(
            variable_decl->pos,
            "Variable declaration is missing both type and expression",
            ERR_UNKNOWN_TYPE
        );
    }
    variable_decl->sema_checked = true;
    // Else nothing to do, type is given but without expression
    return true;
}

/////////////////////////////////////////////////////////////////////////

AstStmt *sem_return(FilePos pos, AstExpr *expr) {
    AstStmt *ret_stmt = ast_return_stmt(pos, ast_return(pos, expr));
    var_pos = pos;

    if (sem_process_stmt(ret_stmt))
        return ret_stmt;

    ast_free_stmt(&ret_stmt);
    return NULL;
}

static bool sem_process_return(AstReturn *return_v) {
    bool void_ret = !(return_v->expr);

    bool ret_expr = ((void_ret) ? true : process_expr(return_v->expr));
    if (!ret_expr)
        // Error already set in process_expr()
        return false;

    if (context.in_func) {
        if (context.func_ret_type == DT_VOID) {
            return sema_err(
                return_v->pos,
                "Unexpected return expression for void type function",
                ERR_INVALID_RETURN
            );
        }

        if (context.func_ret_type != DT_VOID && !return_v->expr) {
            return sema_err(
                return_v->pos,
                "Missing return expression for function",
                ERR_INVALID_RETURN
            );
        }

        context.ret_stmt_found = true;

        if (!return_v->expr->sema_checked)
            return true;

        if (!void_ret)
            check_adapt_to_conversion(return_v->expr, context.func_ret_type);

        // Check for type compatibility (same as for '=')
        if (!void_ret && !compat_array
            [4]
            [get_arr_index(context.func_ret_type)]
            [get_arr_index(return_v->expr->data_type)]
        ) {
            return sema_err(
                return_v->pos,
                "Uncompatible types for function return",
                ERR_INVALID_FUN
            );
        }

        return_v->sema_checked = ((void_ret) ? true : return_v->expr->sema_checked);
    }
    return_v->data_type = ((void_ret) ? DT_VOID : return_v->expr->data_type);

    return true;
}

/////////////////////////////////////////////////////////////////////////

AstCondition *sem_expr_condition(AstExpr *expr) {
    AstCondition *expr_cond = ast_expr_condition(expr);

    if (sem_process_expr_condition(expr_cond->expr)) {
        expr_cond->sema_checked = expr_cond->expr->sema_checked;
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
        // Error already set in process_expr()
        return false;
    // Check if given expression becomes only TRUE/FALSE
    if (expr->type != AST_BINARY_OP) {
        return sema_err(
            expr->pos,
            "Statement condition cant be evaluated",
            ERR_INCOM_TYPE
        );
    }

    // Check for using undefined variables in condition

    // Check for relational operators usage (f.e. operator '+' doesnt make sense and cant be evaluated as true/false)
    Token oper = expr->binary_op->operator;
    // T_EQUALS, T_DIFFERS, T_LESS_OR_EQUAL, T_GREATER_OR_EQUAL, '>', '<'
    if ((oper < T_EQUALS || oper > T_GREATER_OR_EQUAL)
        && oper != '<'
        && oper != '>'
    ) {
        return sema_err(
            expr->binary_op->pos,
            "Statement condition cant be evaluated",
            ERR_INCOM_TYPE
        );
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////

void sem_if_block_end(AstCondition *cond) {
    if (cond->type == AST_COND_LET) {
        if (cond->let->var.counter)
            cond->let->var.counter--;
        else
            cond->let->var.data_type = cond->let->var.original_data_type;
    }
}

/////////////////////////////////////////////////////////////////////////

AstCondition *sem_let_condition(FilePos pos, SymItem *ident) {
    AstCondition *let_cond = ast_let_condition(pos, ident);
    var_pos = pos;

    if (sem_process_let_condition(let_cond->let)) {
        let_cond->sema_checked = true;
        return let_cond;
    }

    ast_free_condition(&let_cond);
    return NULL;
}

static bool sem_process_let_condition(SymItem *ident) {
    if (!ident->declared) {
        return sema_err(
            var_pos,
            "Undeclared variable",
            ERR_UNDEF_FUNCTION
        );
    }
    if (ident->type != SYM_VAR) {
        return sema_err(
            var_pos,
            "Invalid type of ident provided",
            ERR_SEMANTIC
        );
    }

    if (ident->var.mutable) {
        return sema_err(
            var_pos,
            "Mutable variable provided",
            ERR_SEMANTIC
        );
    }

    // If first entry to stmt -> change var type if needed
    if (ident->var.original_data_type == DT_NONE) {
        ident->var.counter = 0;
        // Store original data type
        ident->var.original_data_type = ident->var.data_type;
        // Switch to NOT_NIL values if not already
        if (ident->var.data_type & DT_NIL) {
            // Clear nillable flag - XOR
            ident->var.data_type ^= DT_NIL;
        }
    }
    else
        ident->var.counter++;

    return true;
}

/////////////////////////////////////////////////////////////////////////

static void set_block_checked(AstBlock* block, bool* sema_to_check) {
    *sema_to_check = true;

    VEC_FOR_EACH(&(block->stmts), AstStmt*, stmt) {
        if (!(*stmt.v)->sema_checked)
            *sema_to_check = false;
    }
}

/////////////////////////////////////////////////////////////////////////

static bool not_another_func_decl(Vec stmts) {
    // Check for func_decl in this if's body
    VEC_FOR_EACH(&stmts, AstStmt*, stmt) {
        if ((*stmt.v)->type == AST_FUNCTION_DECL) {
            return sema_err(
                (*stmt.v)->pos,
                "Cant declare function here",
                ERR_SEMANTIC
            );
        }
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////

AstStmt *sem_if(
    FilePos pos,
    AstCondition *cond,
    AstBlock *true_block,
    AstBlock *false_block
) {
    AstStmt *if_stmt = ast_if_stmt(
        pos,
        ast_if(pos, cond, true_block, false_block)
    );
    var_pos = pos;

    if (sem_process_stmt(if_stmt))
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

    if_v->condition->sema_checked = if_v->condition->expr->sema_checked;

    // Process if (and) else body
    bool if_body = sem_process_block(if_v->if_body->stmts);
    if (!if_body)
        // Error already set in sem_block()
        return false;
    // Check for func_decl in this if's body
    if (!not_another_func_decl(if_v->if_body->stmts))
        return false;

    set_block_checked(if_v->if_body, &(if_v->sema_checked));
    if_v->sema_checked = if_v->sema_checked && if_v->condition->sema_checked;

    if (if_v->else_body) {
        bool else_body = sem_process_block(if_v->else_body->stmts);
        if (!else_body)
            // Error already set in sem_block()
            return false;
        // In case of ifbody not checked, avoid potencial override
        if (if_v->sema_checked)
            set_block_checked(if_v->else_body, &(if_v->sema_checked));
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////

AstStmt *sem_expr_stmt(AstExpr *expr) {
    if (sem_expr(expr))
        return ast_expr_stmt(expr);
    return NULL;
}

/////////////////////////////////////////////////////////////////////////

AstStmt *sem_while(FilePos pos, AstCondition *cond, AstBlock *loop) {
    AstStmt *while_stmt = ast_while_stmt(pos, ast_while(pos, cond, loop));
    var_pos = pos;

    if (sem_process_stmt(while_stmt))
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
        return sema_err(while_v->pos, "Missing while() body", ERR_SEMANTIC);
    // Process cycle body
    bool while_body = sem_process_block(while_v->body->stmts);
    if (!while_body)
        // Error already set in sem_block()
        return false;
    // Check for func_decl in this while's body
    if (!not_another_func_decl(while_v->body->stmts))
        return false;

    set_block_checked(while_v->body, &(while_v->sema_checked));
    while_v->sema_checked = while_v->sema_checked && while_v->condition->sema_checked;

    return true;
}

/////////////////////////////////////////////////////////////////////////

bool sem_func_param(String label, SymItem *ident, DataType type, FuncParam *res) {
    ident->var.data_type = type;
    *res = sym_func_param_new(ident, label);
    return true;
}

/////////////////////////////////////////////////////////////////////////

bool sem_lex_literal(Lexer *lex, AstLiteral *res) {
    if (lex->cur != T_LITERAL)
        return false;

    res->data_type = lex->subtype;
    res->pos = lex->token_start;
    res->sema_checked = true;

    if (lex->subtype == DT_INT)
        res->int_v = lex->i_num;
    else if (lex->subtype == DT_DOUBLE)
        res->double_v = lex->d_num;
    else if (lex->subtype == DT_STRING)
        res->string_v = str_clone(lex->str);

    return true;
}
