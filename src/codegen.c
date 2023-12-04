#include "codegen.h"

#include <assert.h>

// helper macro, expects that there is FILE named out
#define OPRINTLN(fmt, ...) fprintf(out, fmt"\n", ##__VA_ARGS__)
// helper macro, expects that there is FILE named out
#define OPRINT(fmt, ...) fprintf(out, fmt,##__VA_ARGS__)

#define STACK_SYMB INST_SYMB_ID(NULL)

// simple error propagation: returns false if the expression is false
#define CHECK(...) if (!(__VA_ARGS__)) return
// simle error propagation: declares variable and returns false if it is not
// true
#define CHECKD(type, name, ...) \
    type name = (__VA_ARGS__); \
    if (!name) return false

bool todo_symb_is_global(SymItem *item);
// makes all name unique
void todo_sym_gen_unique_names(Symtable *sym);
// generates unique temprary variable with the given type
SymItem *todo_sym_tmp_var(Symtable *sym, DataType type);
// generates temorary function (same as `todo_sym_tmp_var` but desn't require
// type, and sets the type to function)
SymItem *todo_sym_label(Symtable *sym);

static bool cg_generate_block(Symtable *sym, Vec code, FILE *out);
static bool cg_generate_inst(Symtable *sym, Instruction inst, FILE *out);
static bool cg_write_symb(InstSymb symb, FILE *out);
static bool cg_write_ident(SymItem *ident, FILE *out);
static bool cg_get_value(InstSymb src, SymItem *dst, FILE *out);
static bool cg_get_symb(Symtable *sym, InstSymb src, InstSymb *dst, FILE *out);
static SymItem *cg_get_ident(Symtable *sym, SymItem *dst);
static bool cg_push(SymItem *dst, FILE *out);
static bool cg_gen_inst3(
    const char *isnt,
    SymItem *dst,
    InstSymb first,
    InstSymb second,
    FILE *out
);

static bool cg_esc_string(String str, FILE *out);
static bool cg_is_stack(InstSymb symb);

static bool cg_gen_binary(
    Symtable *sym,
    const char *inst,
    InstBinary bin,
    FILE *out
);
static bool cg_gen_ifn(Symtable *sym, InstSymb src, SymItem *dst, FILE *out);
static bool cg_gen_cmp_jmp(
    Symtable *sym,
    bool (*cmp)(Symtable *, InstBinary, FILE *),
    InstJmpBin bin,
    FILE *out
);
static bool cg_gen_ncmp_jmp(
    Symtable *sym,
    bool (*cmp)(Symtable *, InstBinary, FILE *),
    InstJmpBin bin,
    FILE *out
);

static bool cg_gen_move(Symtable *sym, InstMove move, FILE *out);
static bool cg_gen_decl(Symtable *sym, InstDecl decl, FILE *out);
static bool cg_gen_call(Symtable *sym, InstCall call, FILE *out);
static bool cg_gen_return(Symtable *sym, InstReturn return_v, FILE *out);
static bool cg_gen_add(Symtable *sym, InstBinary add, FILE *out);
static bool cg_gen_mul(Symtable *sym, InstBinary mul, FILE *out);
static bool cg_gen_div(Symtable *sym, InstBinary div, FILE *out);
static bool cg_gen_lt(Symtable *sym, InstBinary lt, FILE *out);
static bool cg_gen_gt(Symtable *sym, InstBinary gt, FILE *out);
static bool cg_gen_eq(Symtable *sym, InstBinary eq, FILE *out);
static bool cg_gen_lte(Symtable *sym, InstBinary lte, FILE *out);
static bool cg_gen_gte(Symtable *sym, InstBinary gte, FILE *out);
static bool cg_gen_sub(Symtable *sym, InstBinary sub, FILE *out);
static bool cg_gen_neq(Symtable *sym, InstBinary neq, FILE *out);
static bool cg_gen_idiv(Symtable *sym, InstBinary idiv, FILE *out);
static bool cg_gen_concat(Symtable *sym, InstBinary concat, FILE *out);
static bool cg_gen_isnil(Symtable *sym, InstIfNil isnil, FILE *out);
static bool cg_gen_notnil(Symtable *sym, InstIfNil notnil, FILE *out);
static bool cg_gen_label(Symtable *sym, InstLabel label, FILE *out);
static bool cg_gen_jump(Symtable *sym, InstLabel jump, FILE *out);
static bool cg_gen_jif(Symtable *sym, InstLabel jif, FILE *out);
static bool cg_gen_jifn(Symtable *sym, InstLabel jifn, FILE *out);
static bool cg_gen_jeq(Symtable *sym, InstJmpBin jeq, FILE *out);
static bool cg_gen_jneq(Symtable *sym, InstJmpBin jneq, FILE *out);
static bool cg_gen_jgt(Symtable *sym, InstJmpBin jgt, FILE *out);
static bool cg_gen_jlt(Symtable *sym, InstJmpBin jlt, FILE *out);
static bool cg_gen_jlte(Symtable *sym, InstJmpBin jlte, FILE *out);
static bool cg_gen_jgte(Symtable *sym, InstJmpBin jgte, FILE *out);
static bool cg_gen_jisnil(Symtable *sym, InstJmpNil jisnil, FILE *out);
static bool cg_gen_jnonil(Symtable *sym, InstJmpNil jnonil, FILE *out);
static bool cg_gen_exit(Symtable *sym, InstExit exit, FILE *out);

static bool cg_call_read(
    Symtable *sym,
    const char *type,
    InstCall call,
    FILE *out
);
static bool cg_call_write(Symtable *sym, InstCall call, FILE *out);
static bool cg_call_int2double(Symtable *sym, InstCall call, FILE *out);
static bool cg_call_double2int(Symtable *sym, InstCall call, FILE *out);
static bool cg_call_length(Symtable *sym, InstCall call, FILE *out);
static bool cg_call_substring(Symtable *sym, InstCall call, FILE *out);
static bool cg_call_ord(Symtable *sym, InstCall call, FILE *out);
static bool cg_call_chr(Symtable *sym, InstCall call, FILE *out);
static bool cg_call(Symtable *sym, InstCall call, FILE *out);

bool cg_generate(Symtable *sym, InnerCode *code, FILE *out) {
    OPRINTLN(".IFJcode23");

    CHECK(cg_generate_block(sym, code->code, out));

    VEC_FOR_EACH(&code->functions, FunctionCode, fcode) {
        OPRINTLN("");
        OPRINTLN("");
        OPRINTLN("# Function");
        CHECK(cg_generate_block(sym, fcode.v->code, out));
    }

    return true;
}

static bool cg_generate_block(Symtable *sym, Vec code, FILE *out) {
    VEC_FOR_EACH(&code, Instruction, inst) {
        OPRINTLN("");
        CHECK(cg_generate_inst(sym, *inst.v, out));
    }
}

static bool cg_generate_inst(Symtable *sym, Instruction inst, FILE *out) {
    switch (inst.type) {
    case IT_MOVE:
        return cg_gen_move(sym, inst.move, out);
    case IT_DECL:
        return cg_gen_decl(sym, inst.decl, out);
    case IT_CALL:
        return cg_gen_call(sym, inst.call, out);
    case IT_RETURN:
        return cg_gen_return(sym, inst.return_v, out);
    case IT_SUB:
        return cg_gen_sub(sym, inst.binary, out);
    case IT_ADD:
        return cg_gen_add(sym, inst.binary, out);
    case IT_CONCAT:
        return cg_gen_concat(sym, inst.binary, out);
    case IT_MUL:
        return cg_gen_mul(sym, inst.binary, out);
    case IT_DIV:
        return cg_gen_div(sym, inst.binary, out);
    case IT_IDIV:
        return cg_gen_idiv(sym, inst.binary, out);
    case IT_LT:
        return cg_gen_lt(sym, inst.binary, out);
    case IT_GT:
        return cg_gen_gt(sym, inst.binary, out);
    case IT_EQ:
        return cg_gen_eq(sym, inst.binary, out);
    case IT_ISNIL:
        return cg_gen_isnil(sym, inst.if_nil, out);
    case IT_LTE:
        return cg_gen_lte(sym, inst.binary, out);
    case IT_GTE:
        return cg_gen_gte(sym, inst.binary, out);
    case IT_NEQ:
        return cg_gen_neq(sym, inst.binary, out);
    case IT_NOTNIL:
        return cg_gen_notnil(sym, inst.if_nil, out);
    case IT_LABEL:
        return cg_gen_label(sym, inst.label, out);
    case IT_JUMP:
        return cg_gen_jump(sym, inst.label, out);
    case IT_JIF:
        return cg_gen_jif(sym, inst.label, out);
    case IT_JIFN:
        return cg_gen_jifn(sym, inst.label, out);
    case IT_JEQ:
        return cg_gen_jeq(sym, inst.jmp_bin, out);
    case IT_JNEQ:
        return cg_gen_jneq(sym, inst.jmp_bin, out);
    case IT_JLT:
        return cg_gen_jlt(sym, inst.jmp_bin, out);
    case IT_JGT:
        return cg_gen_jgt(sym, inst.jmp_bin, out);
    case IT_JISNIL:
        return cg_gen_jisnil(sym, inst.jmp_nil, out);
    case IT_JLTE:
        return cg_gen_jlte(sym, inst.jmp_bin, out);
    case IT_JGTE:
        return cg_gen_jgte(sym, inst.jmp_bin, out);
    case IT_JNONIL:
        return cg_gen_jnonil(sym, inst.jmp_nil, out);
    case IT_EXIT:
        return cg_gen_exit(sym, inst.exit, out);
    }
}

static bool cg_write_symb(InstSymb symb, FILE *out) {
    if (symb.type == IS_IDENT) {
        return cg_write_ident(symb.ident, out);
    }

    switch (symb.type & DT_TYPE_M) {
    case DT_INT:
        OPRINT("int@%d", symb.literal.int_v);
        break;
    case DT_DOUBLE:
        OPRINT("float@%a", symb.literal.double_v);
        break;
    case DT_STRING:
        OPRINT("string@");
        CHECK(cg_esc_string(symb.literal.str, out));
        break;
    default:
        OPRINT("nil@nil");
        break;
    }

    return true;
}

static bool cg_write_ident(SymItem *ident, FILE *out) {
    if (!ident) {
        assert(false);
        return false;
    }

    if (todo_symb_is_global(ident)) {
        OPRINT("GF@%s", ident->name);
        return true;
    }

    OPRINT("LF@%s", ident->name);
    return true;
}

static bool cg_get_value(InstSymb src, SymItem *dst, FILE *out) {
    // pop from stack
    if (cg_is_stack(src)) {
        // move from top of the stack to the top of the stack is noop
        if (!dst) {
            return true;
        }
        OPRINT("POPS ");
        CHECK(cg_write_ident(dst, out));
        OPRINTLN("");
    }

    if (dst) {
        // move to variable
        OPRINT("MOVE ");
        CHECK(cg_write_ident(dst, out));
        OPRINT(", ");
    } else {
        // move to stack
        OPRINT("PUSHS ");
    }

    CHECK(cg_write_symb(src, out));
    OPRINTLN("");
}

static bool cg_get_symb(Symtable *sym, InstSymb src, InstSymb *dst, FILE *out)
{
    if (!cg_is_stack(src)) {
        *dst = src;
        return true;
    }

    CHECKD(SymItem *, itm, todo_sym_tmp_var(sym, DT_NONE));

    OPTINT("POPS ");
    CHECK(cg_write_ident(itm, out));
    OPRINTLN("");

    *dst = INST_SYMB_ID(itm);
}

static SymItem *cg_get_ident(Symtable *sym, SymItem *dst) {
    return dst ? dst : todo_sym_tmp_var(sym, DT_NONE);
}

static bool cg_push(SymItem *dst, FILE *out) {
    if (dst) {
        return true;
    }

    OPRINT("PUSHS ");
    CHECK(cg_write_ident(dst, out));
    OPRINTLN("");
    return true;
}

static bool cg_gen_inst3(
    const char *inst,
    SymItem *dst,
    InstSymb first,
    InstSymb second,
    FILE *out
) {
    OPRINT("%s ", inst);
    CHECK(cg_write_ident(dst, out));
    OPRINT(", ");
    CHECK(cg_write_symb(first, out));
    OPRINT(", ");
    CHECK(cg_write_symb(second, out));
    OPRINTLN("");
    return true;
}

static bool cg_esc_string(String str, FILE *out) {
    for (; str.len--; ++str.str) {
        unsigned char chr = *str.str;
        if (chr <= 32 || chr == 35 || chr == 92) {
            OPRINT("\\%03hhu", chr);
        } else {
            OPRINT("%c", chr);
        }
    }
    return true;
}

static bool cg_is_stack(InstSymb symb) {
    return symb.type == IS_IDENT && !symb.ident;
}

static bool cg_gen_binary(
    Symtable *sym,
    const char *inst,
    InstBinary bin,
    FILE *out
) {
    if (cg_is_stack(bin.first) && !cg_is_stack(bin.second)) {
        CHECK(cg_get_value(bin.second, NULL, out));
    }

    if (cg_is_stack(bin.first)) {
        OPRINTLN("%sS", inst);
        return cg_get_value(STACK_SYMB, bin.dst, out);
    }

    InstSymb first;
    InstSymb second;
    CHECK(cg_get_symb(sym, bin.first, &first, out));
    CHECK(cg_get_symb(sym, bin.second, &second, out));

    CHECKD(SymItem *, dst, cg_get_ident(sym, bin.dst));
    CHECK(cg_gen_inst3(inst, dst, first, second, out));

    return cg_push(bin.dst, out);
}

static bool cg_gen_ifn(Symtable *sym, InstSymb src, SymItem *dst, FILE *out) {
    if (cg_is_stack(src)) {
        OPRINTLN("PUSHS bool@false");
        OPRINTLN("EQS");
        return cg_get_value(STACK_SYMB, dst, out);
    }

    CHECKD(SymItem *, tdst, cg_get_ident(sym, dst));

    OPRINT("EQ ");
    CHECK(cg_write_ident(tdst, out));
    OPRINT(", ");
    CHECK(cg_write_symb(src, out));
    OPRINTLN(", bool@false");

    return cg_push(dst, out);
}

static bool cg_gen_cmp_jmp(
    Symtable *sym,
    bool (*cmp)(Symtable *, InstBinary, FILE *),
    InstJmpBin bin,
    FILE *out
) {
    InstBinary b = {
        .dst = NULL,
        .first = bin.first,
        .second = bin.second,
    };
    CHECK(cmp(sym, b, out));
    return cg_gen_jif(sym, (InstLabel) { .ident = bin.label }, out);
}

static bool cg_gen_ncmp_jmp(
    Symtable *sym,
    bool (*cmp)(Symtable *, InstBinary, FILE *),
    InstJmpBin bin,
    FILE *out
) {
    InstBinary b = {
        .dst = NULL,
        .first = bin.first,
        .second = bin.second,
    };
    CHECK(cmp(sym, b, out));
    return cg_gen_jifn(sym, (InstLabel) { .ident = bin.label }, out);
}

static bool cg_gen_move(Symtable *sym, InstMove move, FILE *out) {
    return cg_get_value(move.src, move.dst, out);
}

static bool cg_gen_decl(Symtable *sym, InstDecl decl, FILE *out) {
    OPRINT("DECL ");
    CHECK(cg_write_ident(decl.var, out));
    OPRINTLN("");
    return true;
}

static bool cg_gen_call(Symtable *sym, InstCall call, FILE *out) {
    String name = call.ident->name;

    if (str_eq(name, STR("readString"))) {
        return cg_call_read(sym, "string", call, out);
    } else if (str_eq(name, STR("readInt"))) {
        return cg_call_read(sym, "int", call, out);
    } else if (str_eq(name, STR("readDouble"))) {
        return cg_call_read(sym, "float", call, out);
    } else if (str_eq(name, STR("write"))) {
        return cg_call_write(sym, call, out);
    } else if (str_eq(name, STR("Int2Double"))) {
        return cg_call_int2double(sym, call, out);
    } else if (str_eq(name, STR("Double2Int"))) {
        return cg_call_double2int(sym, call, out);
    } else if (str_eq(name, STR("length"))) {
        return cg_call_length(sym, call, out);
    } else if (str_eq(name, STR("substring"))) {
        return cg_call_substring(sym, call, out);
    } else if (str_eq(name, STR("ord"))) {
        return cg_call_ord(sym, call, out);
    } else if (str_eq(name, STR("chr"))) {
        return cg_call_chr(sym, call, out);
    }

    return cg_call(sym, call, out);
}

static bool cg_gen_return(Symtable *sym, InstReturn return_v, FILE *out) {
    if (return_v.value.has_value) {
        CHECK(cg_get_value(return_v.value.value, NULL, out));
    }

    OPRINTLN("POPFRAME");
    OPRINTLN("RETURN");
    return true;
}

static bool cg_gen_add(Symtable *sym, InstBinary add, FILE *out) {
    return cg_gen_binary(sym, "ADD", add, out);
}

static bool cg_gen_mul(Symtable *sym, InstBinary mul, FILE *out) {
    return cg_gen_binary(sym, "MUL", mul, out);
}

static bool cg_gen_div(Symtable *sym, InstBinary div, FILE *out) {
    return cg_gen_binary(sym, "DIV", div, out);
}

static bool cg_gen_lt(Symtable *sym, InstBinary lt, FILE *out) {
    return cg_gen_binary(sym, "LT", lt, out);
}

static bool cg_gen_gt(Symtable *sym, InstBinary gt, FILE *out) {
    return cg_gen_binary(sym, "GT", gt, out);
}

static bool cg_gen_eq(Symtable *sym, InstBinary eq, FILE *out) {
    return cg_gen_binary(sym, "EQ", eq, out);
}

static bool cg_gen_lte(Symtable *sym, InstBinary lte, FILE *out) {
    InstBinary gt = lte;
    gt.dst = NULL;
    CHECK(cg_gen_binary(sym, "GT", gt, out));
    return cg_gen_ifn(sym, STACK_SYMB, lte.dst, out);
}

static bool cg_gen_gte(Symtable *sym, InstBinary gte, FILE *out) {
    InstBinary lt = gte;
    lt.dst = NULL;
    CHECK(cg_gen_binary(sym, "LT", lt, out));
    return cg_gen_ifn(sym, STACK_SYMB, gte.dst, out);
}

static bool cg_gen_sub(Symtable *sym, InstBinary sub, FILE *out) {
    return cg_gen_binary(sym, "SUB", sub, out);
}

static bool cg_gen_neq(Symtable *sym, InstBinary neq, FILE *out) {
    InstBinary eq = neq;
    eq.dst = NULL;
    CHECK(cg_gen_binary(sym, "EQ", eq, out));
    return cg_gen_ifn(sym, STACK_SYMB, neq.dst, out);
}

static bool cg_gen_idiv(Symtable *sym, InstBinary idiv, FILE *out) {
    return cg_gen_binary(sym, "IDIV", idiv, out);
}

static bool cg_gen_concat(Symtable *sym, InstBinary concat, FILE *out) {
    InstSymb second;
    InstSymb first;
    CHECK(cg_get_symb(sym, concat.second, &second, out));
    CHECK(cg_get_symb(sym, concat.first, &first, out));
    CHECKD(SymItem *, dst, cg_get_ident(sym, concat.dst));

    CHECK(cg_gen_inst3("CONCAT", dst, first, second, out));
    return cg_push(concat.dst, out);
}

static bool cg_gen_isnil(Symtable *sym, InstIfNil isnil, FILE *out) {
    CHECK(cg_get_value(INST_SYMB_ID(isnil.src), NULL, out));
    OPRINTLN("PUSHS nil@nil");
    OPRINTLN("EQS");
    return cg_get_value(STACK_SYMB, isnil.dst, out);
}

static bool cg_gen_notnil(Symtable *sym, InstIfNil notnil, FILE *out) {
    InstIfNil isnil = notnil;
    isnil.dst = NULL;
    CHECK(cg_gen_isnil(sym, isnil, out));
    return cg_gen_ifn(sym, STACK_SYMB, isnil.dst, out);
}

static bool cg_gen_label(Symtable *sym, InstLabel label, FILE *out) {
    OPRINT("LABEL %s", label.ident->name.str);
    return true;
}

static bool cg_gen_jump(Symtable *sym, InstLabel jump, FILE *out) {
    OPRINT("JUMP %s", jump.ident->name.str);
    return true;
}

static bool cg_gen_jif(Symtable *sym, InstLabel jif, FILE *out) {
    OPRINTLN("PUSH bool@true");
    OPRINTLN("JUMPIFEQ %s", jif.ident->name.str);
    return true;
}

static bool cg_gen_jifn(Symtable *sym, InstLabel jifn, FILE *out) {
    OPRINTLN("PUSH bool@true");
    OPRINTLN("JUMPIFNEQ %s", jifn.ident->name.str);
    return true;
}

static bool cg_gen_jeq(Symtable *sym, InstJmpBin jeq, FILE *out) {
    return cg_gen_cmp_jmp(sym, cg_gen_eq, jeq, out);
}

static bool cg_gen_jneq(Symtable *sym, InstJmpBin jneq, FILE *out) {
    return cg_gen_ncmp_jmp(sym, cg_gen_eq, jneq, out);
}

static bool cg_gen_jgt(Symtable *sym, InstJmpBin jgt, FILE *out) {
    return cg_gen_cmp_jmp(sym, cg_gen_gt, jgt, out);
}

static bool cg_gen_jlt(Symtable *sym, InstJmpBin jlt, FILE *out) {
    return cg_gen_cmp_jmp(sym, cg_gen_lt, jlt, out);
}

static bool cg_gen_jlte(Symtable *sym, InstJmpBin jlte, FILE *out) {
    return cg_gen_ncmp_jmp(sym, cg_gen_gt, jlte, out);
}

static bool cg_gen_jgte(Symtable *sym, InstJmpBin jgte, FILE *out) {
    return cg_gen_ncmp_jmp(sym, cg_gen_lt, jgte, out);
}

static bool cg_gen_jisnil(Symtable *sym, InstJmpNil jisnil, FILE *out) {
    CHECK(cg_get_value(INST_SYMB_ID(jisnil.src), NULL, out));
    OPRINTLN("PUSHS nil@nil");
    OPRINTLN("JUMPIFEQS %s", jisnil.label->name.str);
    return true;
}

static bool cg_gen_jnonil(Symtable *sym, InstJmpNil jnonil, FILE *out) {
    CHECK(cg_get_value(INST_SYMB_ID(jnonil.src), NULL, out));
    OPRINTLN("PUSHS nil@nil");
    OPRINTLN("JUMPIFNEQS %s", jnonil.label->name.str);
    return true;
}

static bool cg_gen_exit(Symtable *sym, InstExit exit, FILE *out) {
    OPRINT("EXIT ");
    CHECK(cg_write_symb(exit.value, out));
    OPRINTLN("");
    return true;
}

static bool cg_call_read(
    Symtable *sym,
    const char *type,
    InstCall call,
    FILE *out
);
static bool cg_call_write(Symtable *sym, InstCall call, FILE *out);
static bool cg_call_int2double(Symtable *sym, InstCall call, FILE *out);
static bool cg_call_double2int(Symtable *sym, InstCall call, FILE *out);
static bool cg_call_length(Symtable *sym, InstCall call, FILE *out);
static bool cg_call_substring(Symtable *sym, InstCall call, FILE *out);
static bool cg_call_ord(Symtable *sym, InstCall call, FILE *out);
static bool cg_call_chr(Symtable *sym, InstCall call, FILE *out);
static bool cg_call(Symtable *sym, InstCall call, FILE *out);
