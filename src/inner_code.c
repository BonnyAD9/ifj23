#include "inner_code.h"

#include "assert.h"

void todo_sym_gen_unique_names(Symtable *sym);
SymItem *todo_sym_tmp_var(Symtable *sym, DataType type);
SymItem *todo_sym_label(Symtable *sym);

// extracts functions from the top level block
// vector of AstFunctionDecl *
static Vec ic_get_blocks(AstBlock *block);

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
    InstOptIdent dst,
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
    todo_sym_gen_unique_names(sym);
    Vec funcs = ic_get_blocks(block);
    res->functions = VEC_NEW(FunctionCode);

    VEC_FOR_EACH(&funcs, AstFunctionDecl *, fun) {
        Vec body = VEC_NEW(Instruction);
        if (!ic_gen_block(sym, (*fun.v)->body, &body)) {
            ast_free_block(&block);
            vec_free_with(&res->functions, (FreeFun)ic_free_func_code);
            vec_free_with(&funcs, (FreeFun)ast_free_function_decl);
            return false;
        }

        FunctionCode func = {
            .code = body,
            .ident = (*fun.v)->ident,
        };
    }

    res->code = VEC_NEW(Instruction);
    if (!ic_gen_block(sym, block, &res->code)) {
        ast_free_block(&block);
        vec_free_with(&res->functions, (FreeFun)ic_free_func_code);
        vec_free_with(&funcs, (FreeFun)ast_free_function_decl);
        return false;
    }

    ast_free_block(&block);
    vec_free_with(&funcs, (FreeFun)ast_free_function_decl);

    return true;
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
    if (symb->type == IS_LITERAL && symb->literal.type == DT_STRING) {
        str_free(&symb->literal.str);
    }
}

void ic_free_opt_symb(InstOptSymb *symb) {
    if (symb->has_value) {
        ic_free_symb(&symb->value);
    }
    symb->has_value = false;
}

static Vec ic_get_blocks(AstBlock *block) {
    Vec funcs = VEC_NEW(AstFunctionDecl *);

    VEC_FOR_EACH(&block->stmts, AstStmt *, stmt) {
        if ((*stmt.v)->type != AST_FUNCTION_DECL) {
            continue;
        }

        vec_push(&funcs, stmt.v);
        vec_remove(&block->stmts, stmt.i);
        --stmt.i;
    }

    return funcs;
}

static bool ic_gen_block(Symtable *sym, AstBlock *block, Vec *code) {
    VEC_FOR_EACH(&block->stmts, AstStmt *, stmt) {
        ic_gen_stmt(sym, *stmt.v, code);
    }
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
        SymItem *tmp = todo_sym_tmp_var(sym, op->left->data_type);
        if (!tmp) {
            return false;
        }
        inst = (Instruction) {
            .type = IT_DECL,
            .decl = { .var = tmp }
        };
        if (!vec_push(code, &inst)) {
            return false;
        }

        // MOVE tmp, eval(op->left)
        if (!ic_gen_expr(sym, op->left, INST_IDENT(tmp), code)) {
            return false;
        }

        SymItem *l_nil = todo_sym_label(sym);
        if (!l_nil) {
            return false;
        }

        SymItem *l_end = todo_sym_label(sym);
        if (!l_end) {
            return false;
        }

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

        if (!vec_push_span(code, SPAN_ARR(insts))) {
            return false;
        }

        // MOVE dst, eval(op->right)
        if (!ic_gen_expr(sym, op->right, dst, code)) {
            return false;
        }

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

    // PUSH eval(op->left)
    // PUSH eval(op->right)
    // OP top1(stack), top(stack)

    // PUSH eval(op->left)
    if (!ic_gen_expr(sym, op->left, INST_IDENT(NULL), code)) {
        return false;
    }

    // PUSH eval(op->right)
    if (!ic_gen_expr(sym, op->right, INST_IDENT(NULL), code)) {
        return false;
    }

    // OP dst, top1(stack), top(stack)
    inst = (Instruction) {
        .binary = {
            .dst = dst.ident,
            .first = NULL,
            .second = NULL,
        },
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
        inst.type = IT_DIV;
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
        // if this happens, it is bug
        assert(false);
        return false;
    }
    return vec_push(code, &inst);
}

static bool ic_gen_unary(
    Symtable *sym,
    AstUnaryOp *op,
    InstOptIdent dst,
    Vec *code
) {
    if (op == '!' || op == '+' || !dst.has_value) {
        // MOVE dst, eval(op->param)
        return ic_gen_expr(sym, op->param, dst, code);
    }

    assert(op == '-');

    // PUSH 0
    // PUSH eval(op->param)
    // SUB top1(stack), top(stack)

    // PUSH 0
    InstLiteral zero = { .type = DT_INT, .int_v = 0 };
    if (op->param->data_type == DT_DOUBLE) {
        zero = (InstLiteral) { .type = DT_DOUBLE, .double_v = 0 };
    }
    Instruction inst = {
        .type = IT_MOVE,
        .move = { .dst = NULL, .src = INST_SYMB_LIT(zero) },
    };
    if (!vec_push(sym, &inst)) {
        return false;
    }

    // PUSH eval(op->param)
    if (!ic_gen_expr(sym, op->param, INST_STACK_IDENT, code)) {
        return false;
    }

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
    };
    if (!vec_push(code, &inst)) {
        vec_free_with(&args, (FreeFun)ic_free_symb);
        return false;
    }
    return true;
}

static InstSymb symb_from_literal(AstLiteral *lit) {
    InstSymb res = { .type = IS_LITERAL };

    switch (lit->data_type) {
    case DT_INT:
        res.literal.int_v = lit->int_v;
        break;
    case DT_DOUBLE:
        res.literal.double_v = lit->double_v;
        break;
    case DT_STRING:
        res.literal.str = lit->string_v;
        // ensure that the string is not double-freed
        lit->data_type = DT_INT;
        break;
    default:
        assert(false);
    }

    return res;
}
