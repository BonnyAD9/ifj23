/**
 * IFJ23
 *
 * xstigl00 Jakub Antonín Štigler
 */

#include "inner_code.h"

#include "errors.h"
// extracts functions from the top level block
// vector of AstFunctionDecl *
static bool ic_get_blocks(AstBlock *block, Vec *res);

static bool ic_gen_block(Symtable *sym, AstBlock *block, Vec *code);
static bool ic_gen_binary(
    Symtable *sym,
    AstBinaryOp *op,
    InstOptIdent dst,
    Vec *code
);
static bool ic_gen_unary(
    Symtable *sym,
    AstUnaryOp *op,
    InstOptIdent dst,
    Vec *code
);
static bool ic_gen_literal(
    Symtable *sym,
    AstLiteral *op,
    InstOptIdent dst,
    Vec *code
);
static bool ic_gen_call(
    Symtable *sym,
    AstFunctionCall *call,
    InstOptIdent dst,
    Vec *code
);
static bool ic_gen_return(
    Symtable *sym,
    AstReturn *ret,
    Vec *code
);
static bool ic_gen_var_decl(Symtable *sym, AstVariableDecl *decl, Vec *code);
static bool ic_gen_condition(
    Symtable *sym,
    AstCondition *cond,
    SymItem *label,
    Vec *code
);
static bool ic_gen_if(Symtable *sym, AstIf *if_v, Vec *code);
static bool ic_gen_while(Symtable *sym, AstWhile *while_v, Vec *code);
static bool ic_gen_variable(
    Symtable *sym,
    AstVariable *var,
    InstOptIdent dst,
    Vec *code
);
static bool ic_gen_expr(
    Symtable *sym,
    AstExpr *expr,
    InstOptIdent dst,
    Vec *code
);
static bool ic_gen_stmt(Symtable *sym, AstStmt *stmt, Vec *code);

static InstSymb symb_from_literal(AstLiteral *lit);

bool ic_inner_code(Symtable *sym, AstBlock *block, InnerCode *res) {
    CHECK(sym_scope_add(sym));
    Vec funcs = VEC_NEW(AstFunctionDecl *);
    if (!ic_get_blocks(block, &funcs)) {
        vec_free_with(&funcs, (FreeFun)ast_free_function_decl);
        ast_free_block(&block);
        return false;
    }
    res->functions = VEC_NEW(FunctionCode);

    VEC_FOR_EACH(&funcs, AstFunctionDecl *, fun) {
        Vec body = VEC_NEW(Instruction);

        // (*fun.v)->ident:
        Instruction inst = {
            .type = IT_LABEL,
            .label = { .ident = (*fun.v)->ident },
            .pos = (*fun.v)->ident->file_pos,
        };

        if (!vec_push(&body, &inst)
            || !ic_gen_block(sym, (*fun.v)->body, &body)
        ) {
            ast_free_block(&block);
            vec_free_with(&res->functions, (FreeFun)ic_free_func_code);
            vec_free_with(&funcs, (FreeFun)ast_free_function_decl);
            return false;
        }

        FunctionCode func = {
            .code = body,
            .ident = (*fun.v)->ident,
        };
        if (!vec_push(&res->functions, &func)) {
            ast_free_block(&block);
            vec_free_with(&res->functions, (FreeFun)ic_free_func_code);
            vec_free_with(&funcs, (FreeFun)ast_free_function_decl);
            return false;
        }
    }

    res->code = VEC_NEW(Instruction);

    // EXIT 0
    Instruction inst = {
        .type = IT_EXIT,
        .exit = {
            .value = {
                .type = IS_LITERAL,
                .literal = { .type = DT_INT, .int_v = 0 },
            },
        }
    };

    if (!ic_gen_block(sym, block, &res->code)
        || !vec_push(&res->code, &inst)
    ) {
        ast_free_block(&block);
        vec_free_with(&res->functions, (FreeFun)ic_free_func_code);
        vec_free_with(&funcs, (FreeFun)ast_free_function_decl);
        return false;
    }

    ast_free_block(&block);
    vec_free_with(&funcs, (FreeFun)ast_free_function_decl);

    return true;
}

void ic_free_code(InnerCode *code) {
    vec_free_with(&code->code, (FreeFun)ic_free_instruction);
    vec_free_with(&code->functions, (FreeFun)ic_free_func_code);
}

void ic_free_func_code(FunctionCode *code) {
    vec_free_with(&code->code, (FreeFun)ic_free_instruction);
}

void ic_free_instruction(Instruction *inst) {
    switch (inst->type) {
    case IT_MOVE:
        ic_free_symb(&inst->move.src);
        break;
    case IT_CALL:
        vec_free_with(&inst->call.params, (FreeFun)ic_free_symb);
        break;
    case IT_RETURN:
        ic_free_opt_symb(&inst->return_v.value);
        break;
    case IT_ADD:
    case IT_MUL:
    case IT_DIV:
    case IT_LT:
    case IT_GT:
    case IT_EQ:
    case IT_LTE:
    case IT_GTE:
        ic_free_symb(&inst->binary.first);
        ic_free_symb(&inst->binary.second);
        break;
    case IT_JEQ:
    case IT_JNEQ:
    case IT_JGT:
    case IT_JLT:
    case IT_JLTE:
    case IT_JGTE:
        ic_free_symb(&inst->jmp_bin.first);
        ic_free_symb(&inst->jmp_bin.second);
        break;
    case IT_EXIT:
        ic_free_symb(&inst->exit.value);
        break;
    default:
        break;
    }
}

void ic_free_symb(InstSymb *symb) {
    if (symb->type == IS_LITERAL
        && ((symb->literal.type & DT_TYPE_M) == DT_STRING))
    {
        str_free(&symb->literal.str);
    }
}

void ic_free_opt_symb(InstOptSymb *symb) {
    if (symb->has_value) {
        ic_free_symb(&symb->value);
    }
    symb->has_value = false;
}

static bool ic_get_blocks(AstBlock *block, Vec *res) {
    VEC_FOR_EACH(&block->stmts, AstStmt *, stmt) {
        if ((*stmt.v)->type != AST_FUNCTION_DECL) {
            continue;
        }

        CHECK(vec_push(res, &(*stmt.v)->function_decl));
        free(*stmt.v);
        vec_remove(&block->stmts, stmt.i);
        --stmt.i;
    }

    return true;
}

static bool ic_gen_block(Symtable *sym, AstBlock *block, Vec *code) {
    VEC_FOR_EACH(&block->stmts, AstStmt *, stmt) {
        if (!ic_gen_stmt(sym, *stmt.v, code)) {
            return false;
        }
    }
    return true;
}

static bool ic_gen_binary(
    Symtable *sym,
    AstBinaryOp *op,
    InstOptIdent dst,
    Vec *code
) {
    Instruction inst;

    if (op->operator == T_DOUBLE_QUES) {
        //   DECL tmp
        //   MOVE tmp, eval(op->left)
        //   JISNIL tmp, l_nil
        //   MOVE dst, tmp
        //   JUMP l_end
        // l_nil:
        //   MOVE dst, eval(op->right)
        // l_end:

        // DECL tmp
        CHECKD(SymItem *, tmp, sym_tmp_var(sym, op->left->data_type));

        inst = (Instruction) {
            .type = IT_DECL,
            .decl = { .var = tmp },
            .pos = op->pos,
        };
        CHECK(vec_push(code, &inst));

        // MOVE tmp, eval(op->left)
        CHECK(ic_gen_expr(sym, op->left, INST_IDENT(tmp), code));

        CHECKD(SymItem *, l_nil, sym_label(sym));
        CHECKD(SymItem *, l_end, sym_label(sym));

        Instruction insts[] = {
            { // JISNIL tmp, l_nil
                .type = IT_JISNIL,
                .jmp_nil = { .label = l_nil, .src = tmp }
            },
            { // MOVE dst, tmp
                .type = IT_MOVE,
                .move = { .dst = dst.ident, .src = INST_SYMB_ID(tmp) }
            },
            { // JUMP l_end
                .type = IT_JUMP,
                .label = { .ident = l_end }
            },
            { // l_nil:
                .type = IT_LABEL,
                .label = { .ident = l_nil }
            }
        };

        CHECK(vec_push_span(code, SPAN_ARR(insts)));

        // MOVE dst, eval(op->right)
        CHECK(ic_gen_expr(sym, op->right, dst, code));

        // l_end:
        inst = (Instruction) {
            .type = IT_LABEL,
            .label = {
                .ident = l_end,
            },
        };
        return vec_push(code, &inst);
    }

    if (op->operator == '=') {
        // MOVE op->left->variable->ident, eval(op->right)
        return ic_gen_expr(
            sym,
            op->right,
            INST_IDENT(op->left->variable->ident),
            code
        );
    }

    if (!dst.has_value) {
        // eval(op->left)
        // eval(op->right)
        return ic_gen_expr(sym, op->left, INST_NONE_IDENT, code)
            && ic_gen_expr(sym, op->right, INST_NONE_IDENT, code);
    }

    if (op->operator == '+' && (
        (op->left->data_type & DT_STRING)
        || op->right->data_type & DT_STRING)
    ) {
        // DECL tmp1
        // DECL tmp2
        // MOVE tmp1, eval(op->left)
        // MOVE tmp2, eval(op->right)
        // CONCAT dst, tmp1, tmp2

        CHECKD(SymItem *, tmp1, sym_tmp_var(sym, DT_STRING));
        CHECKD(SymItem *, tmp2, sym_tmp_var(sym, DT_STRING));

        Instruction insts[] = {
            { // DECL tmp1
                .type = IT_DECL,
                .decl = { .var = tmp1 },
                .pos = op->pos,
            },
            { // DECL tmp2
                .type = IT_DECL,
                .decl = { .var = tmp2 },
            }
        };

        CHECK(vec_push_span(code, SPAN_ARR(insts)));

        // MOVE tmp1, eval(op->left)
        CHECK(ic_gen_expr(sym, op->left, INST_IDENT(tmp1), code));

        // MOVE tmp2, eval(op->right)
        CHECK(ic_gen_expr(sym, op->right, INST_IDENT(tmp2), code));

        // CONCAT dst, tmp1, tmp2
        Instruction inst = {
            .type = IT_CONCAT,
            .binary = {
                .dst = dst.ident,
                .first = INST_SYMB_ID(tmp1),
                .second = INST_SYMB_ID(tmp2),
            },
        };
        return vec_push(code, &inst);
    }

    // PUSH eval(op->left)
    // PUSH eval(op->right)
    // OP top1(stack), top(stack)

    inst = (Instruction) {
        .binary = {
            .dst = dst.ident,
            .first = INST_SYMB_ID(NULL),
            .second = INST_SYMB_ID(NULL),
        },
        .pos = op->pos,
    };

    switch ((int)op->operator) {
    case '+':
        inst.type = IT_ADD;
        break;
    case '-':
        inst.type = IT_SUB;
        break;
    case '*':
        inst.type = IT_MUL;
        break;
    case '/':
        inst.type = (op->left->data_type & DT_INT)
                || (op->right->data_type & DT_INT)
            ? IT_IDIV
            : IT_DIV;
        break;
    case '<':
        inst.type = IT_LT;
        break;
    case '>':
        inst.type = IT_GT;
        break;
    case T_EQUALS:
        inst.type = IT_EQ;
        break;
    case T_DIFFERS:
        inst.type = IT_NEQ;
        break;
    case T_LESS_OR_EQUAL:
        inst.type = IT_LTE;
        break;
    case T_GREATER_OR_EQUAL:
        inst.type = IT_GTE;
        break;
    default:
        return OTHER_ERR_FALSE;
    }

    // PUSH eval(op->left)
    CHECK(ic_gen_expr(sym, op->left, INST_IDENT(NULL), code));

    // PUSH eval(op->right)
    CHECK(ic_gen_expr(sym, op->right, INST_IDENT(NULL), code));

    // OP dst, top1(stack), top(stack)
    return vec_push(code, &inst);
}

static bool ic_gen_unary(
    Symtable *sym,
    AstUnaryOp *op,
    InstOptIdent dst,
    Vec *code
) {
    if (op->operator == '!' || op->operator == '+' || !dst.has_value) {
        // MOVE dst, eval(op->param)
        return ic_gen_expr(sym, op->param, dst, code);
    }

    if (op->operator != '-') {
        return OTHER_ERR_FALSE;
    }

    // PUSH 0
    // PUSH eval(op->param)
    // SUB top1(stack), top(stack)

    // PUSH 0
    InstLiteral zero = { .type = DT_INT, .int_v = 0 };
    if (op->param->data_type & DT_DOUBLE) {
        zero = (InstLiteral) { .type = DT_DOUBLE, .double_v = 0 };
    }
    Instruction inst = {
        .type = IT_MOVE,
        .move = { .dst = NULL, .src = INST_SYMB_LIT(zero) },
        .pos = op->pos,
    };
    CHECK(vec_push(code, &inst));

    // PUSH eval(op->param)
    CHECK(ic_gen_expr(sym, op->param, INST_STACK_IDENT, code));

    // SUB dst, top1(stack), top(stack)
    inst = (Instruction) {
        .type = IT_SUB,
        .binary = {
            .dst = dst.ident,
            .first = INST_SYMB_ID(NULL),
            .second = INST_SYMB_ID(NULL),
        },
    };
    return vec_push(code, &inst);
}

static bool ic_gen_literal(
    Symtable *sym,
    AstLiteral *lit,
    InstOptIdent dst,
    Vec *code
) {
    if (!dst.has_value) {
        return true;
    }

    // MOVE dst, lit

    Instruction inst = {
        .type = IT_MOVE,
        .move = {
            .dst = dst.ident,
            .src = symb_from_literal(lit),
        },
        .pos = lit->pos,
    };

    return vec_push(code, &inst);
}

static bool ic_gen_call(
    Symtable *sym,
    AstFunctionCall *call,
    InstOptIdent dst,
    Vec *code
) {
    Vec args = VEC_NEW(InstSymb);

    // MOVE dst, call->ident(call->arguments)

    VEC_FOR_EACH(&call->arguments, AstFuncCallParam, param) {
        InstSymb arg;
        if (param.v->type == AST_VARIABLE) {
            arg.type = IS_IDENT;
            arg.ident = param.v->variable;
        } else {
            arg = symb_from_literal(&param.v->literal);
        }
        if (!vec_push(&args, &arg)) {
            vec_free_with(&args, (FreeFun)ic_free_symb);
            return false;
        }
    }

    Instruction inst = {
        .type = IT_CALL,
        .call = {
            .dst = dst,
            .ident = call->ident,
            .params = args,
        },
        .pos = call->pos,
    };
    if (!vec_push(code, &inst)) {
        vec_free_with(&args, (FreeFun)ic_free_symb);
        return false;
    }
    return true;
}

static bool ic_gen_return(
    Symtable *sym,
    AstReturn *ret,
    Vec *code
) {
    // MOVE tmp, eval(ret->expr)
    // RETURN tmp

    // MOVE tmp, eval(ret->expr)
    InstOptIdent tmp = { .has_value = false };
    if (ret->expr) {
        tmp.has_value = true;
        tmp.ident = NULL;
        CHECK(ic_gen_expr(sym, ret->expr, tmp, code));
    }

    // RETURN tmp
    Instruction inst = {
        .type = IT_RETURN,
        .return_v = {
            .value = {
                .has_value = tmp.has_value,
                .value = { .type = IS_IDENT, .ident = tmp.ident },
            },
        },
        .pos = ret->pos,
    };
    return vec_push(code, &inst);
}

static bool ic_gen_var_decl(Symtable *sym, AstVariableDecl *decl, Vec *code) {
    // DECL decl->ident
    // MOVE decl->ident, decl->value

    // DECL decl->ident
    Instruction inst = {
        .type = IT_DECL,
        .decl = { .var = decl->ident },
        .pos = decl->pos,
    };
    CHECK(vec_push(code, &inst));

    // MOVE decl->ident, decl->value
    if (decl->value) {
        CHECK(ic_gen_expr(sym, decl->value, INST_IDENT(decl->ident), code));
    }

    return true;
}

static bool ic_gen_condition(
    Symtable *sym,
    AstCondition *cond,
    SymItem *label,
    Vec *code
) {
    if (cond->type == AST_COND_LET) {
        // JISNIL iv_v->condition->let, label
        Instruction inst = {
            .type = IT_JISNIL,
            .jmp_nil = { .src = cond->let, .label = label },
            .pos = cond->pos,
        };
        return vec_push(code, &inst);
    }

    // PUSH eval(cond->expr)
    // JIFN top(stack), label

    // PUSH eval(cond->expr)
    CHECK(ic_gen_expr(sym, cond->expr, INST_IDENT(NULL), code));

    // JIFN top(stack), label
    Instruction inst = {
        .type = IT_JIFN,
        .label = { .ident = label },
        .pos = cond->pos,
    };
    return vec_push(code, &inst);
}

static bool ic_gen_if(Symtable *sym, AstIf *if_v, Vec *code) {
    //   CONDITION if_v->condition, l_false
    //   eval(if_v->if_body)
    //   JUMP l_end
    // l_false:
    //   eval(if_v->else_body)
    // l_end:

    CHECKD(SymItem *, l_false, sym_label(sym));

    // CONDITION if_v->condition, l_false
    CHECK(ic_gen_condition(sym, if_v->condition, l_false, code));

    // eval(cond->if_body)
    CHECK(ic_gen_block(sym, if_v->if_body, code));

    Instruction inst = {
        .type = IT_LABEL,
        .label = { .ident = l_false },
    };

    if (!if_v->else_body) {
        // l_false:
        inst.pos = if_v->pos;
        return vec_push(code, &inst);
    }

    CHECKD(SymItem *, l_end, sym_label(sym));

    Instruction insts[] = {
        { // JUMP l_end
            .type = IT_JUMP,
            .label = { .ident = l_end },
            .pos = if_v->pos,
        },
        // l_false:
        inst,
    };

    CHECK(vec_push_span(code, SPAN_ARR(insts)));

    // eval(if_v->else_body)
    CHECK(ic_gen_block(sym, if_v->else_body, code));

    // l_end:
    inst = (Instruction) {
        .type = IT_LABEL,
        .label = { .ident = l_end },
    };
    return vec_push(code, &inst);
}

static bool ic_gen_while(Symtable *sym, AstWhile *while_v, Vec *code) {
    // l_start:
    //   CONDITION while_v->condition, l_end
    //   eval(while_v->body)
    //   JUMP l_start
    // l_end:

    CHECKD(SymItem *, l_start, sym_label(sym));
    CHECKD(SymItem *, l_end, sym_label(sym));

    Vec block_code = VEC_NEW(Instruction);
    // eval(while_v->body)
    if (!ic_gen_block(sym, while_v->body, &block_code)) {
        vec_free_with(&block_code, (FreeFun)ic_free_instruction);
        return false;
    }

    VEC_FOR_EACH(&block_code, Instruction, inst) {
        if (inst.v->type == IT_DECL) {
            if (!vec_push(code, inst.v)) {
                vec_free_with(&block_code, (FreeFun)ic_free_instruction);
                return false;
            }
        }
    }

    // l_start:
    Instruction inst = {
        .type = IT_LABEL,
        .label = { .ident = l_start },
        .pos = while_v->pos,
    };
    if (!vec_push(code, &inst)) {
        vec_free_with(&block_code, (FreeFun)ic_free_instruction);
        return false;
    }

    // CONDITION while_v->condition, l_end
    if (!ic_gen_condition(sym, while_v->condition, l_end, code)) {
        vec_free_with(&block_code, (FreeFun)ic_free_instruction);
        return false;
    }

    VEC_FOR_EACH(&block_code, Instruction, inst) {
        if (inst.v->type != IT_DECL) {
            if (!vec_push(code, inst.v)) {
                vec_free_with(&block_code, (FreeFun)ic_free_instruction);
                return false;
            }
            // make sure there is no double free
            inst.v->type = IT_DECL;
        }
    }

    vec_free(&block_code);

    Instruction insts[] = {
        { // JUMP l_start
            .type = IT_JUMP,
            .label = { .ident = l_start },
        },
        { // l_end:
            .type = IT_LABEL,
            .label = { .ident = l_end },
        },
    };

    return vec_push_span(code, SPAN_ARR(insts));
}

static bool ic_gen_variable(
    Symtable *sym,
    AstVariable *var,
    InstOptIdent dst,
    Vec *code
) {
    // MOVE dst, var->ident

    if (!dst.has_value) {
        return true;
    }

    // MOVE dst, var->ident
    Instruction inst = {
        .type = IT_MOVE,
        .move = { .dst = dst.ident, .src = INST_SYMB_ID(var->ident) },
        .pos = var->pos,
    };
    return vec_push(code, &inst);
}

static bool ic_gen_expr(
    Symtable *sym,
    AstExpr *expr,
    InstOptIdent dst,
    Vec *code
) {
    // MOVE dst, eval(expr)

    // MOVE dst, eval(expr)
    switch (expr->type) {
    case AST_BINARY_OP:
        return ic_gen_binary(sym, expr->binary_op, dst, code);
    case AST_UNARY_OP:
        return ic_gen_unary(sym, expr->unary_op, dst, code);
    case AST_FUNCTION_CALL:
        return ic_gen_call(sym, expr->function_call, dst, code);
    case AST_LITERAL:
        return ic_gen_literal(sym, expr->literal, dst, code);
    case AST_VARIABLE:
        return ic_gen_variable(sym, expr->variable, dst, code);
    default:
        break;
    }

    return OTHER_ERR_FALSE;
}

static bool ic_gen_stmt(Symtable *sym, AstStmt *stmt, Vec *code) {
    // eval(stmt)

    // eval(stmt)
    switch (stmt->type) {
    case AST_EXPR:
        return ic_gen_expr(sym, stmt->expr, INST_NONE_IDENT, code);
    case AST_BLOCK:
        return ic_gen_block(sym, stmt->block, code);
    case AST_VARIABLE_DECL:
        return ic_gen_var_decl(sym, stmt->variable_decl, code);
    case AST_RETURN:
        return ic_gen_return(sym, stmt->return_v, code);
    case AST_IF:
        return ic_gen_if(sym, stmt->if_v, code);
    case AST_WHILE:
        return ic_gen_while(sym, stmt->while_v, code);
    default:
        break;
    }

    return OTHER_ERR_FALSE;
}

static InstSymb symb_from_literal(AstLiteral *lit) {
    InstSymb res = { .type = IS_LITERAL };

    switch (lit->data_type & DT_TYPE_M) {
    case DT_INT:
        res.literal.type = DT_INT;
        res.literal.int_v = lit->int_v;
        break;
    case DT_DOUBLE:
        res.literal.type = DT_DOUBLE;
        res.literal.double_v = lit->double_v;
        break;
    case DT_STRING:
        res.literal.type = DT_STRING;
        res.literal.str = lit->string_v;
        // ensure that the string is not double-freed
        lit->data_type = DT_NIL;
        break;
    default: // NIL
        res.literal.type = DT_ANY_NIL;
        break;
    }

    return res;
}
