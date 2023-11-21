#include "semantics.h"

static bool sema_err(const char *msg, const int err_type) {
    set_err_code(err_type);
    EPRINTF("%s", msg);
    return false;
}

/////////////////////////////////////////////////////////////////////////
bool          sem_func_param(String label, SymItem *ident, DataType type, FuncParam *res); // DONE

AstBlock     *sem_block(Vec stmts, bool top_level); // DONE

AstCondition *sem_expr_condition(AstExpr *expr); // DONE
AstCondition *sem_let_condition(SymItem *ident); // DONE

AstStmt      *sem_while(AstExpr *cond, AstBlock *loop); // DONE
AstStmt      *sem_var_decl(SymItem *ident, AstExpr *expr); // DONE
AstStmt      *sem_func_decl(SymItem *ident, Vec params, DataType return_type, AstBlock *body); // DONE
AstStmt      *sem_if(AstCondition *cond, AstBlock *true_block, AstBlock *false_block); // DONE
AstStmt      *sem_return(AstExpr *expr); // DONE

AstExpr      *sem_unary(AstExpr *expr, Token op); // DONE
AstExpr      *sem_variable(SymItem *ident); // DONE
AstExpr      *sem_literal(FullToken token); // DONE
AstExpr      *sem_call(AstExpr *calle, Vec params); // DONE
AstExpr      *sem_lex_variable(Lexer *lex, Symtable *table);
AstExpr      *sem_lex_literal(Lexer *lex); // DONE
AstExpr      *sem_binary(AstExpr *left, Token op, AstExpr *right); // DONE

/////////////////////////////////////////////////////////////////////////

AstExpr *sem_expr(AstExpr *expr) {
    DataType type;
    if (process_expr(expr, &type))
        return expr;
    return NULL;
}

static bool sem_process_expr(AstExpr *expr) {
    DataType type;
    return process_expr(expr, &type);
}

static bool process_expr(AstExpr *expr, DataType *final_type) {
    switch (expr->type) {
        case AST_BINARY_OP:
            return sem_process_binary(expr->binary_op, final_type);
        case AST_UNARY_OP:
            return sem_process_unary(expr->unary_op, final_type);
        case AST_FUNCTION_CALL:
            return sem_process_call(expr->function_call, final_type);
        case AST_LITERAL:
            return sem_process_literal(expr->literal, final_type);
        case AST_VARIABLE:
            return sem_process_variable(expr->variable->ident, final_type);
        default:
            return sema_err("Uknown expression type", ERR_SEMANTIC);
    }
}

AstExpr *sem_binary(AstExpr *left, Token op, AstExpr *right) {
    DataType type;
    AstExpr *binary_expr = ast_binary_op_expr(ast_binary_op(op, left, right));
    if (sem_process_binary(binary_expr->binary_op, &type))
        return binary_expr;
    return NULL;
}

static bool sem_process_binary(AstBinaryOp *binary_op, DataType *final_type) {
    // Avoid f.e. funccall(a,b) = 5 type of expressions
    AstExprType left_expr_type = binary_op->left->type;
    if ((left_expr_type == AST_FUNCTION_CALL || left_expr_type == AST_LITERAL) && binary_op->operator == '=')
        return sema_err("Invalid l-value of expression", ERR_SEMANTIC);

    bool left_side = process_expr(binary_op->left, final_type);
    DataType left_side_type = final_type;

    bool right_side = process_expr(binary_op->right, final_type);
    DataType right_side_type = final_type;

    if (!left_side || !right_side)
        // Error is already set by lower instancies
        return false;

    if (check_compatibility(binary_op->operator, left_side_type, right_side_type, final_type))
        return true;
    return false;
}

AstExpr *sem_unary(AstExpr *expr, Token op) {
    DataType type;
    AstExpr *unary_expr = ast_unary_op_expr(ast_unary_op(op, expr));
    if (sem_process_unary(unary_expr->unary_op, &type))
        return unary_expr;
    return NULL;
}

static bool sem_process_unary(AstUnaryOp *unary_op, DataType *final_type) {
    bool eval_expr = process_expr(unary_op->param, final_type);
    // Check for non-translateable options
    if (*final_type == DT_NONE || !eval_expr)
        return sema_err("Invalid expression, cant convert to non-nil type", ERR_SEMANTIC);

    // Only allowed unary operator is '!'
    if (unary_op->operator != '!')
        return sema_err("Invalid unary operator", ERR_SEMANTIC);

    // Switch to NOT_NIL values if not already set
    if (*final_type & DT_NIL)
        // Clear nillable flag - XOR
        *final_type ^= DT_NIL;

    return true;
}
// TODO Discord talks about calle->XXX
AstExpr *sem_call(AstExpr *calle, Vec params) {
    DataType type;
    AstExpr *func_call_expr = ast_function_call_expr(ast_function_call(calle->function_call->ident, params));

    if (sem_process_call(func_call_expr->function_call, &type))
        return func_call_expr;
    return NULL;
}

static bool sem_process_call(AstFunctionCall *func_call, DataType *final_type) {
    if (!func_call->ident->declared)
        return sema_err("Undefined function", ERR_UNDEF_FUNCTION);

    *final_type = sym_func_get_ret(func_call->ident, func_call->ident->name);

    if (check_func_params(func_call->ident, func_call->arguments))
        return true;
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
        AstFuncDeclParam param = VEC_AT(params, AstFuncDeclParam, arg.i);
        DataType param_data_type, arg_data_type;

        // Load also param data type
        if (!sem_process_variable(param.ident, &param_data_type))
            return false;

        if (arg.v->type == AST_VARIABLE) {
            // Check if variable is defined
            if (!sem_process_variable(arg.v->variable, &arg_data_type))
                // Error already set by sem_process_variable()
                return false;
        }
        else if (arg.v->type == AST_LITERAL) {
            if (!sem_process_literal(arg.v->literal, &arg_data_type))
                // Error already set by sem_process_literal()
                return false;
        }
        else
            return sema_err("Unexpected argument type", ERR_INVALID_FUN);

        // Check for correct ident name
        if (strcmp(arg.v->name.str, param.name.str))
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

AstExpr *sem_literal(FullToken token) {
    DataType type;
    AstLiteral *literal;
    if (token.subtype == DT_INT)
        literal = ast_int_literal(token.int_v);
    else if (token.subtype == DT_DOUBLE)
        literal = ast_double_literal(token.double_v);
    else if (token.subtype == DT_STRING)
        literal = ast_string_literal(token.str);
    else // DT_NIL
        literal = ast_nil_literal();

    if (sem_process_literal(literal, &type))
        return ast_literal_expr(literal);
    return NULL;
}

static bool sem_process_literal(AstLiteral *literal, DataType *final_type) {
    // TODO is this conversion really necessary?
    // Implicit conversion to DOUBLE type
    if (literal->type == DT_INT || literal->type == DT_INT_NIL)
        // INT(2) -> DOUBLE(4) , INT_NIL(3) -> DOUBLE_NIL(5)
        *final_type = literal->type + 2;
    return true;
}

AstExpr *sem_variable(SymItem *ident) {
    DataType type;
    AstExpr *var_expr = ast_variable_expr(ast_variable(ident));

    if (sem_process_variable(var_expr->variable->ident, &type))
        return var_expr;
    return NULL;
}

static bool sem_process_variable(SymItem *ident, DataType *final_type) {
    if (!ident->declared)
        return sema_err("Undeclared variable", ERR_UNDEF_FUNCTION);

    if (ident->type != SYM_VAR)
        return sema_err("Invalid type of ident provided", ERR_UNDEF_FUNCTION);

    if (ident->var.data_type & DT_NIL)
        // Throw warning about potencial nil
        WPRINTF("Variable can contain nil value");

    *final_type = ident->var.data_type;
    return true;
}

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
}

static void check_in_array(unsigned int arr_sel, DataType left_type, DataType right_type, const char *err_msg, const char *err_msg_cnt, DataType *final_type) {
    if (!compat_array[arr_sel][get_arr_index(left_type)][get_arr_index(right_type)])
        // Set error message to non-NILL otherwise keep it NULL
        err_msg = err_msg_cnt;
    else
        *final_type = final_type_arr[get_arr_index(left_type)][get_arr_index(right_type)];
}

static bool check_compatibility(Token operator, DataType left_type, DataType right_type, DataType *final_type) {
    const char *err_msg = NULL;
    switch (operator) {
        case '*':
            check_in_array(0, left_type, right_type, err_msg, "Uncompatible types for performing '*' operation", final_type);
            break;
        case '-':
            check_in_array(0, left_type, right_type, err_msg, "Uncompatible types for performing '-' operation", final_type);
            break;
        case '/':
            check_in_array(0, left_type, right_type, err_msg, "Uncompatible types for performing '/' operation", final_type);
            break;
        ////////////////////
        case '+':
            check_in_array(1, left_type, right_type, err_msg, "Uncompatible types for performing '+' operation", final_type);
            break;
        ////////////////////
        case T_EQUALS:           // '=='
            check_in_array(2, left_type, right_type, err_msg, "Uncompatible types for performing '==' operation", final_type);
            break;
        case T_DIFFERS:          // '!='
            check_in_array(2, left_type, right_type, err_msg, "Uncompatible types for performing '!=' operation", final_type);
            break;
        case T_LESS_OR_EQUAL:    // '<='
            check_in_array(2, left_type, right_type, err_msg, "Uncompatible types for performing '<=' operation", final_type);
            break;
        case T_GREATER_OR_EQUAL: // '>='
            check_in_array(2, left_type, right_type, err_msg, "Uncompatible types for performing '>=' operation", final_type);
            break;
        ////////////////////
        case T_DOUBLE_QUES:  // ??
            check_in_array(3, left_type, right_type, err_msg, "Uncompatible types for performing '??' operation", final_type);
            break;
        ////////////////////
        case '=':
            check_in_array(4, left_type, right_type, err_msg, "Uncompatible types for performing '=' operation", final_type);
            break;
        ////////////////////
        case '<':
            check_in_array(5, left_type, right_type, err_msg, "Uncompatible types for performing '<' operation", final_type);
            break;
        case '>':
            check_in_array(5, left_type, right_type, err_msg, "Uncompatible types for performing '>' operation", final_type);
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

AstBlock *sem_block(Vec stmts, bool top_level) {
    AstBlock *block = ast_block(stmts);

    if (sem_process_block(stmts, top_level))
        return block;
    return NULL;
}

static bool sem_process_block(Vec stmts, bool top_level) {
    VEC_FOR_EACH(&(stmts), AstStmt, stmt) {
        bool stmt_valid = sem_process_stmt(stmt.v, top_level);
        if (!stmt_valid)
            // Error already set in sem_process_stmt()
            return false;
    }
    return true;
}

static bool sem_process_stmt(AstStmt *stmt, bool top_level) {
    switch (stmt->type) {
        case AST_EXPR:
            return sem_process_expr(sem_expr(stmt->expr));
        case AST_BLOCK:
            return sem_process_block(stmt->block->stmts, top_level);
        case AST_FUNCTION_DECL:
            if (top_level)
                return sem_process_func_decl(stmt->function_decl);
            return sema_err("Cant declare function in function", ERR_SEMANTIC);
        case AST_VARIABLE_DECL:
            return sem_process_var_decl(stmt->variable_decl);
        case AST_RETURN:
            if (!top_level)
                return sem_process_return(stmt->return_v);
            return sema_err("Return statement in main body is prohibited", ERR_SEMANTIC);
        case AST_IF:
            return sem_process_if(stmt->if_v);
        case AST_WHILE:
            return sem_process_while(stmt->while_v);
        default:
            return sema_err("Uknown stament type", ERR_SEMANTIC);
    }
}

/////////////////////////////////////////////////////////////////////////
// TODO add free for failed sema check
AstStmt *sem_func_decl(SymItem *ident, Vec params, DataType return_type, AstBlock *body) {
    AstStmt *func_decl = ast_function_decl_stmt(ast_function_decl(ident, params, body));

    if (sem_process_func_decl(func_decl->function_decl))
        return func_decl;
    return NULL;
}

bool sem_process_func_decl(AstFunctionDecl *func_decl) {
    if (!func_decl->ident->declared)
        return sema_err("Undeclared function", ERR_UNDEF_FUNCTION);

    if (!check_func_params(func_decl->ident, func_decl->parameters))
        // Error already set in check_func_params()
        return false;

    // First check body is valid
    bool func_body = sem_process_block(func_decl->body->stmts, true);
    if (!func_body)
        // Error already set in sem_block()
        return false;

    // Now iterate through valid body for return statement
    VEC_FOR_EACH(&(func_decl->body->stmts), AstStmt, stmt) {
        if (stmt.v->type == AST_RETURN) {
            // TODO sym_func_get_ret() should return DataType instead of ReturnType
            DataType return_type = sym_func_get_ret(func_decl->ident, func_decl->ident->name);
            if (return_type == DT_VOID && stmt.v->return_v->expr)
                return sema_err("Unexpected return expression for void type function", ERR_INVALID_RETURN);

            if (return_type != DT_VOID && !stmt.v->return_v->expr)
                return sema_err("Missing return expression for function", ERR_INVALID_RETURN);

            DataType expr_type;
            process_expr(stmt.v->return_v->expr, &expr_type);
            // Check for type compatibility (same as for '=')
            if (!compat_array[4][get_arr_index(return_type)][get_arr_index(expr_type)])
                return sema_err("Uncompatible types for function return", ERR_INVALID_FUN);
        }
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////

AstStmt *sem_var_decl(SymItem *ident, AstExpr *expr) {
    AstStmt *var_decl = ast_variable_decl_stmt(ast_variable_decl(ident, expr));

    if (sem_process_var_decl(var_decl->variable_decl))
        return var_decl;
    return NULL;
}

static bool sem_process_var_decl(AstVariableDecl *variable_decl) {
    DataType var_type;
    bool var_expr = sem_process_variable(variable_decl->ident, &var_type);
    if (!var_expr)
        // Error already set in sem_variable()
        return false;

    // Check expression
    if (variable_decl->value) {
        DataType expr_type;
        bool check_expr = process_expr(variable_decl->value, &expr_type);

        // Check if expr type is inferiable
        if (expr_type == DT_ANY_NIL || !check_expr)
            return sema_err("Expression type cant be determined", ERR_UNKNOWN_TYPE);

        // Check if data type is given, or try determine it
        if (variable_decl->ident->var.data_type == DT_NONE)
            variable_decl->ident->var.data_type = expr_type;
        else // var_type value is useless, just avoid using NULL to avoid NULL checks in the function
            return check_compatibility('=', variable_decl->ident->var.data_type, expr_type, &var_type);
    }
    else if (variable_decl->ident->type == DT_NONE)
        // Case when type and expression are missing
        return sema_err("Variable declaration is missing both type and expression", ERR_UNKNOWN_TYPE);
    // Else nothing to do, type is given but without expression
    return true;
}

/////////////////////////////////////////////////////////////////////////

AstStmt *sem_return(AstExpr *expr) {
    AstStmt *ret_stmt = ast_return_stmt(ast_return(expr));

    if (sem_process_return(ret_stmt->return_v))
        return ret_stmt;
    return NULL;
}

static bool sem_process_return(AstReturn *return_v) {
    DataType type;
    bool ret_expr = process_expr(return_v->expr, &type);
    if (!ret_expr)
        // Error already set in sem_expr()
        return false;
    return true;
}

/////////////////////////////////////////////////////////////////////////

AstCondition *sem_expr_condition(AstExpr *expr) {
    AstCondition *expr_cond = ast_expr_condition(expr);

    if (sem_process_expr_condition(expr_cond->expr))
        return expr_cond;
    return NULL;
}

bool sem_process_expr_condition(AstExpr *expr) {
    DataType type;
    // Check if expression is correct
    bool cond_expr = process_expr(expr, &type);
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
    return true;
}

/////////////////////////////////////////////////////////////////////////

AstCondition *sem_let_condition(SymItem *ident) {
    AstCondition *let_cond = ast_let_condition(ident);

    if (sem_process_let_condition(let_cond->let))
        return let_cond;
    return NULL;
}

bool sem_process_let_condition(SymItem *ident) {
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

    if (sem_process_if(if_stmt->if_v))
        return if_stmt;
    return NULL;
}

static bool sem_process_if(AstIf *if_v) {
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
    // TODO how to decide what to use for top_level value
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
    return true;
}

/////////////////////////////////////////////////////////////////////////

AstStmt *sem_while(AstExpr *cond, AstBlock *loop) {
    AstStmt *while_stmt = ast_while_stmt(ast_while(cond, loop));

    if (sem_process_while(while_stmt->while_v))
        return while_stmt;
    return NULL;
}

static bool sem_process_while(AstWhile *while_v) {
    bool while_cond;
    if (while_v->condition->type == AST_COND_EXPR)
        while_cond = sem_process_expr_condition(while_v->condition->expr);
    else // AST_COND_LET
        while_cond = sem_process_let_condition(while_v->condition->let);

    if (!while_cond)
        // Error already set in above func
        return false;

    bool while_body = sem_process_block(while_v->body->stmts, false);
    if (!while_body)
        // Error already set in sem_block()
        return false;
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

AstExpr *sem_lex_variable(Lexer *lex, Symtable *table) {
    lex_next(lex);
    SymItem *ident = sym_find(table, lex->str);
    ident->type = SYM_VAR;
    return ast_variable_expr(ast_variable(ident));
}

/////////////////////////////////////////////////////////////////////////

AstExpr *sem_lex_literal(Lexer *lex) {
    AstLiteral *literal;
    if (lex->subtype == DT_INT)
        literal = ast_int_literal(lex->i_num);
    else if (lex->subtype == DT_DOUBLE)
        literal = ast_double_literal(lex->d_num);
    else if (lex->subtype == DT_STRING)
        literal = ast_string_literal(lex->str);
    else // DT_NIL
        literal = ast_nil_literal();

    return ast_literal_expr(literal);
}
