#include "inner_code.h"

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
    InstOptSymb value,
    Vec *code
);
static bool ic_gen_unary(
    Symtable *sym,
    AstBinaryOp *op,
    InstOptSymb value,
    Vec *code
);
static bool ic_gen_literal(
    Symtable *sym,
    AstLiteral *op,
    InstOptSymb value,
    Vec *code
);
static bool ic_gen_call(
    Symtable *sym,
    AstFunctionCall *call,
    InstOptSymb value,
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
    InstOptSymb value,
    Vec *code
);
static bool ic_gen_if(Symtable *sym, AstIf *if_v, Vec *code);
static bool ic_gen_while(Symtable *sym, AstWhile *while_v, Vec *code);
static bool ic_gen_variable(
    Symtable *sym,
    AstVariable *var,
    InstOptSymb value,
    Vec *code
);
static bool ic_gen_expr(
    Symtable *sym,
    AstExpr *expr,
    InstOptSymb value,
    Vec *code
);
static bool ic_gen_stmt(Symtable *sym, AstStmt *stmt, Vec *code);

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

static bool ic_gen_binary(Symtable *sym, AstBinaryOp *op, InstOptSymb value, Vec *code) {
    if (!value.has_value && op->operator != T_DOUBLE_QUES) {
        ic_gen_expr(sym, op->left, INST_NONE_SYMB, code);
        ic_gen_expr(sym, op->right, INST_NONE_SYMB, code);
        return;
    }


}
