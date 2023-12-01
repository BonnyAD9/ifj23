#include "printer.h"

#include <stdio.h>

static void indent(size_t amount) {
    while (amount--) {
        printf("    ");
    }
}

void print_data_type(DataType type) {
    if (type == DT_NONE) {
        printf("DataType(NONE)");
        return;
    }

    if (type == DT_VOID) {
        printf("DataType(VOID)");
        return;
    }

    printf("DataType(");

    if ((type & DT_TYPE_M) == DT_ANY) {
        printf("ANY ");
    } else {
        if (type & DT_INT) {
            printf("INT ");
        }
        if (type & DT_DOUBLE) {
            printf("DOUBLE ");
        }
        if (type & DT_STRING) {
            printf("STRING ");
        }
    }

    if (type & DT_NIL) {
        printf("NIL ");
    }

    if (type & DT_VOID) {
        printf("VOID ");
    }

    printf(")");
}

void print_token(Token token) {
    printf("Token(");
    switch (token) {
    case T_ERR:
        printf("ERR");
        break;
    case T_EOF:
        printf("EOF");
        break;
    case T_IDENT:
        printf("IDENT");
        break;
    case T_LITERAL:
        printf("LITERAL");
        break;
    case T_RETURNS:
        printf("->");
        break;
    case T_EQUALS:
        printf("==");
        break;
    case T_DIFFERS:
        printf("!=");
        break;
    case T_LESS_OR_EQUAL:
        printf("<=");
        break;
    case T_GREATER_OR_EQUAL:
        printf(">=");
        break;
    case T_DOUBLE_QUES:
        printf("??");
        break;
    case T_FUNC:
        printf("func");
        break;
    case T_IF:
        printf("if");
        break;
    case T_ELSE:
        printf("else");
        break;
    case T_WHILE:
        printf("while");
        break;
    case T_DECL:
        printf("DECL");
        break;
    case T_TYPE:
        printf("TYPE");
        break;
    case T_UNARY_MINUS:
        printf("UNARY_MINUS");
        break;
    case T_UNARY_PLUS:
        printf("UNARY_PLUS");
        break;
    case T_EXPR_PAREN:
        printf("EXPR_PAREN");
        break;
    default:
        printf("%c", (char)token);
        break;
    }
    printf(")");
}

void print_ast_block(AstBlock *block, size_t depth) {
    printf("Block {\n");
    VEC_FOR_EACH(&block->stmts, AstStmt *, i) {
        indent(depth);
        print_ast_stmt(*i.v, depth + 1);
        printf(",\n");
    }
    indent(depth - 1);
    printf("}");
}

void print_ast_binary_op(AstBinaryOp *op, size_t depth) {
    printf("BinaryOp(");
    print_token(op->operator);
    printf(") {\n");
    indent(depth);
    printf("left {");
    print_ast_expr(op->left, depth + 1);
    printf("},\n");
    indent(depth);
    printf("right {");
    print_ast_expr(op->right, depth + 1);
    printf("}\n");
    indent(depth - 1);
    printf("}");
}

void print_ast_unary_op(AstUnaryOp *op, size_t depth) {
    printf("UnaryOp(");
    print_token(op->operator);
    printf(") {");
    print_ast_expr(op->param, depth);
    printf("}");
}

void print_ast_literal(AstLiteral *l) {
    printf("Literal(");
    switch (l->data_type) {
    case DT_ANY_NIL:
        printf("nil");
        break;
    case DT_INT:
        printf("Int(%d)", l->int_v);
        break;
    case DT_DOUBLE:
        printf("Double(%lf)", l->double_v);
        break;
    case DT_STRING:
        printf("String(%s)", l->string_v.str);
        break;
    default:
        printf("INVALID");
        break;
    }
    printf(")");
}

void print_ast_func_call_param(AstFuncCallParam *par, size_t depth) {
    printf("FuncCallParam{");
    if (par->name.str) {
        printf("%s", par->name.str);
    } else {
        printf("_");
    }

    printf(" ");

    if (par->type == AST_LITERAL) {
        print_ast_literal(&par->literal);
    } else {
        printf("%s", par->variable->name.str);
    }

    printf("}");
}

void print_ast_function_call(AstFunctionCall *call, size_t depth) {
    printf("FunctionCall(%s) {\n", call->ident->name.str);
    VEC_FOR_EACH(&call->arguments, AstFuncCallParam, i) {
        indent(depth);
        print_ast_func_call_param(i.v, depth + 1);
        printf(",\n");
    }
    indent(depth - 1);
    printf("}");
}

void print_ast_function_decl(AstFunctionDecl *decl, size_t depth) {
    printf("FunctionDecl(%s) {", decl->ident->name.str);
    print_ast_block(decl->body, depth);
    printf("}");
}

void print_ast_return(AstReturn *ret, size_t depth) {
    printf("Return {");
    print_ast_expr(ret->expr, depth);
    printf("}");
}

void print_ast_variable_decl(AstVariableDecl *decl, size_t depth) {
    printf("VariableDecl(%s) {", decl->ident->name.str);
    if (decl->value) {
        print_ast_expr(decl->value, depth);
    }
    printf("}");
}

void print_ast_condition(AstCondition *cond, size_t depth) {
    printf("Condition {");
    if (cond->type == AST_COND_EXPR) {
        print_ast_expr(cond->expr, depth);
    } else {
        printf("Let(%s)", cond->let->name.str);
    }
    printf("}");
}

void print_ast_if(AstIf *if_v, size_t depth) {
    printf("If {\n");
    indent(depth);
    printf("condition {");
    print_ast_condition(if_v->condition, depth + 1);
    printf("},\n");
    indent(depth);
    printf("true {");
    print_ast_block(if_v->if_body, depth + 1);
    printf("},\n");
    if (if_v->else_body) {
        indent(depth);
        printf("false {");
        print_ast_block(if_v->else_body, depth + 1);
        printf("}\n");
    }
    indent(depth - 1);
    printf("}");
}

void print_ast_while(AstWhile *while_v, size_t depth) {
    printf("While {\n");
    indent(depth);
    printf("condition {");
    print_ast_condition(while_v->condition, depth + 1);
    printf("},\n");
    indent(depth);
    printf("loop {");
    print_ast_block(while_v->body, depth + 1);
    printf("},\n");
    indent(depth - 1);
    printf("}");
}

void print_ast_variable(AstVariable *var) {
    printf("Variable(%s)", var->ident->name.str);
}

void print_ast_expr(AstExpr *expr, size_t depth) {
    printf("Expr{");
    switch (expr->type) {
    case AST_BINARY_OP:
        print_ast_binary_op(expr->binary_op, depth);
        break;
    case AST_UNARY_OP:
        print_ast_unary_op(expr->unary_op, depth);
        break;
    case AST_FUNCTION_CALL:
        print_ast_function_call(expr->function_call, depth);
        break;
    case AST_LITERAL:
        print_ast_literal(expr->literal);
        break;
    case AST_VARIABLE:
        print_ast_variable(expr->variable);
        break;
    default:
        printf("INVALID");
        break;
    }
    printf("}");
}

void print_ast_stmt(AstStmt *stmt, size_t depth) {
    printf("Stmt {");
    switch (stmt->type) {
    case AST_EXPR:
        print_ast_expr(stmt->expr, depth);
        break;
    case AST_BLOCK:
        print_ast_block(stmt->block, depth);
        break;
    case AST_FUNCTION_DECL:
        print_ast_function_decl(stmt->function_decl, depth);
        break;
    case AST_VARIABLE_DECL:
        print_ast_variable_decl(stmt->variable_decl, depth);
        break;
    case AST_RETURN:
        print_ast_return(stmt->return_v, depth);
        break;
    case AST_IF:
        print_ast_if(stmt->if_v, depth);
        break;
    case AST_WHILE:
        print_ast_while(stmt->while_v, depth);
        break;
    default:
        printf("INVALID");
        break;
    }
    printf("}");
}

