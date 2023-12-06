#include "codegen.h"

#include <assert.h>

#include "debug_tools.h"
#include "errors.h"
#include "errno.h"

// helper macro, expects that there is FILE named out
#define OPRINTLN(fmt, ...) if ((fprintf(out, fmt"\n", ##__VA_ARGS__), errno)) \
    return false
// helper macro, expects that there is FILE named out
#define OPRINT(fmt, ...) if ((fprintf(out, fmt,##__VA_ARGS__), errno)) \
    return false

#define STACK_SYMB INST_SYMB_ID(NULL)

static bool cg_generate_block(Symtable *sym, Vec code, FILE *out);
static bool cg_generate_inst(Symtable *sym, Instruction inst, FILE *out);
static bool cg_write_symb(InstSymb symb, FILE *out);
static bool cg_write_tsymb(InstSymb symb, bool tf, FILE *out);
static bool cg_write_ident(SymItem *ident, FILE *out);
static bool cg_write_tident(SymItem *ident, bool tf, FILE *out);
static bool cg_get_value(InstSymb src, SymItem *dst, FILE *out);
static bool cg_get_tvalue(InstSymb src, SymItem *dst, bool tf, FILE *out);
static bool cg_get_symb(Symtable *sym, InstSymb src, InstSymb *dst, FILE *out);
static SymItem *cg_get_ident(Symtable *sym, SymItem *dst, FILE *out);
static bool cg_push(SymItem *cur, SymItem *target, FILE *out);
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

static bool cg_single_arg(
    Symtable *sym,
    const char *inst,
    InstCall call,
    FILE *out
);

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
    OPRINTLN("");
    OPRINTLN("CREATEFRAME");
    OPRINTLN("PUSHFRAME");

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
    return true;
}

static bool cg_generate_inst(Symtable *sym, Instruction inst, FILE *out) {
    if (inst.pos.line) {
        OPRINTLN("# " DEBUG_FILE ":%zu:%zu:", inst.pos.line, inst.pos.column);
    }
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

    return OTHER_ERR_FALSE;
}

static bool cg_write_symb(InstSymb symb, FILE *out) {
    return cg_write_tsymb(symb, false, out);
}

static bool cg_write_tsymb(InstSymb symb, bool tf, FILE *out) {
    if (symb.type == IS_IDENT) {
        return cg_write_tident(symb.ident, tf, out);
    }

    switch (symb.literal.type & DT_TYPE_M) {
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
    return cg_write_tident(ident, false, out);
}

static bool cg_write_tident(SymItem *ident, bool tf, FILE *out) {
    if (!ident) {
        return OTHER_ERR_FALSE;
    }

    if (tf) {
        OPRINT("TF@%s", ident->uname.str);
        return true;
    }

    if (ident->global) {
        OPRINT("GF@%s", ident->uname.str);
        return true;
    }

    OPRINT("LF@%s", ident->uname.str);
    return true;
}

static bool cg_get_value(InstSymb src, SymItem *dst, FILE *out) {
    return cg_get_tvalue(src, dst, false, out);
}
static bool cg_get_tvalue(InstSymb src, SymItem *dst, bool tf, FILE *out) {
    // pop from stack
    if (cg_is_stack(src)) {
        // move from top of the stack to the top of the stack is noop
        if (!dst) {
            return true;
        }
        OPRINT("POPS ");
        CHECK(cg_write_tident(dst, tf, out));
        OPRINTLN("");
        return true;
    }

    if (dst) {
        // move to variable
        OPRINT("MOVE ");
        CHECK(cg_write_tident(dst, tf, out));
        OPRINT(" ");
    } else {
        // move to stack
        OPRINT("PUSHS ");
    }

    CHECK(cg_write_symb(src, out));
    OPRINTLN("");

    return true;
}

static bool cg_get_symb(Symtable *sym, InstSymb src, InstSymb *dst, FILE *out)
{
    if (!cg_is_stack(src)) {
        *dst = src;
        return true;
    }

    CHECKD(SymItem *, itm, sym_tmp_var(sym, DT_NONE));
    OPRINT("DEFVAR ");
    CHECK(cg_write_ident(itm, out));
    OPRINTLN("");

    OPRINT("POPS ");
    CHECK(cg_write_ident(itm, out));
    OPRINTLN("");

    *dst = INST_SYMB_ID(itm);

    return true;
}

static SymItem *cg_get_ident(Symtable *sym, SymItem *dst, FILE *out) {
    if (dst) {
        return dst;
    }
    dst = sym_tmp_var(sym, DT_NONE);
    if (!dst) {
        return NULL;
    }

    OPRINT("DEFVAR ");
    if (cg_write_ident(dst, out)) {
        return NULL;
    }
    OPRINTLN("");
    return dst;
}

static bool cg_push(SymItem *cur, SymItem *target, FILE *out) {
    if (target) {
        return true;
    }

    OPRINT("PUSHS ");
    CHECK(cg_write_ident(cur, out));
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
    OPRINT(" ");
    CHECK(cg_write_symb(first, out));
    OPRINT(" ");
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

    CHECKD(SymItem *, dst, cg_get_ident(sym, bin.dst, out));
    CHECK(cg_gen_inst3(inst, dst, first, second, out));

    return cg_push(dst, bin.dst, out);
}

static bool cg_gen_ifn(Symtable *sym, InstSymb src, SymItem *dst, FILE *out) {
    if (cg_is_stack(src)) {
        OPRINTLN("PUSHS bool@false");
        OPRINTLN("EQS");
        return cg_get_value(STACK_SYMB, dst, out);
    }

    CHECKD(SymItem *, tdst, cg_get_ident(sym, dst, out));

    OPRINT("EQ ");
    CHECK(cg_write_ident(tdst, out));
    OPRINT(" ");
    CHECK(cg_write_symb(src, out));
    OPRINTLN(" bool@false");

    return cg_push(tdst, dst, out);
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
    OPRINT("DEFVAR ");
    CHECK(cg_write_ident(decl.var, out));
    OPRINTLN("");
    return true;
}

static bool cg_gen_call(Symtable *sym, InstCall call, FILE *out) {
    String name = call.ident->uname;

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
    CHECKD(SymItem *, dst, cg_get_ident(sym, concat.dst, out));

    CHECK(cg_gen_inst3("CONCAT", dst, first, second, out));
    return cg_push(dst, concat.dst, out);
}

static bool cg_gen_isnil(Symtable *sym, InstIfNil isnil, FILE *out) {
    CHECK(cg_get_value(INST_SYMB_ID(isnil.src), NULL, out));
    OPRINTLN("PUSHS nil@nil");
    OPRINTLN("EQS ");
    return cg_get_value(STACK_SYMB, isnil.dst, out);
}

static bool cg_gen_notnil(Symtable *sym, InstIfNil notnil, FILE *out) {
    InstIfNil isnil = notnil;
    isnil.dst = NULL;
    CHECK(cg_gen_isnil(sym, isnil, out));
    return cg_gen_ifn(sym, STACK_SYMB, isnil.dst, out);
}

static bool cg_gen_label(Symtable *sym, InstLabel label, FILE *out) {
    OPRINT("LABEL %s", label.ident->uname.str);
    return true;
}

static bool cg_gen_jump(Symtable *sym, InstLabel jump, FILE *out) {
    OPRINT("JUMP %s", jump.ident->uname.str);
    return true;
}

static bool cg_gen_jif(Symtable *sym, InstLabel jif, FILE *out) {
    OPRINTLN("PUSHS bool@true");
    OPRINTLN("JUMPIFEQS %s", jif.ident->uname.str);
    return true;
}

static bool cg_gen_jifn(Symtable *sym, InstLabel jifn, FILE *out) {
    OPRINTLN("PUSHS bool@true");
    OPRINTLN("JUMPIFNEQS %s", jifn.ident->uname.str);
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
    OPRINTLN("JUMPIFEQS %s", jisnil.label->uname.str);
    return true;
}

static bool cg_gen_jnonil(Symtable *sym, InstJmpNil jnonil, FILE *out) {
    CHECK(cg_get_value(INST_SYMB_ID(jnonil.src), NULL, out));
    OPRINTLN("PUSHS nil@nil");
    OPRINTLN("JUMPIFNEQS %s", jnonil.label->uname.str);
    return true;
}

static bool cg_gen_exit(Symtable *sym, InstExit exit, FILE *out) {
    OPRINT("EXIT ");
    CHECK(cg_write_symb(exit.value, out));
    OPRINTLN("");
    return true;
}

static bool cg_single_arg(
    Symtable *sym,
    const char *inst,
    InstCall call,
    FILE *out
) {
    if (!call.dst.has_value) {
        return true;
    }

    InstSymb *args = &VEC_AT(&call.params, InstSymb, 0);

    if (call.dst.ident) {
        OPRINT("%s ", inst);
        CHECK(cg_write_ident(call.dst.ident, out));
        OPRINT(" ");
        CHECK(cg_write_symb(*args, out));
        OPRINTLN("");
        return true;
    }

    OPRINT("PUSHS ");
    CHECK(cg_write_symb(*args, out));
    OPRINTLN("");

    OPRINTLN("%sS", inst);

    return true;
}

static bool cg_call_read(
    Symtable *sym,
    const char *type,
    InstCall call,
    FILE *out
) {
    SymItem *dst = call.dst.has_value ? call.dst.ident : NULL;
    CHECK(dst = cg_get_ident(sym, dst, out));

    OPRINT("READ ");
    CHECK(cg_write_ident(dst, out));
    OPRINTLN(" %s", type);

    return call.dst.has_value ? cg_push(dst, call.dst.ident, out) : true;
}

static bool cg_call_write(Symtable *sym, InstCall call, FILE *out) {
    VEC_FOR_EACH(&call.params, InstSymb, arg) {
        OPRINT("WRITE ");
        CHECK(cg_write_symb(*arg.v, out));
        OPRINTLN("");
    }

    return true;
}

static bool cg_call_int2double(Symtable *sym, InstCall call, FILE *out) {
    OPRINTLN("# int2double");
    return cg_single_arg(sym, "INT2FLOAT", call, out);
}

static bool cg_call_double2int(Symtable *sym, InstCall call, FILE *out) {
    OPRINTLN("# double2int");
    return cg_single_arg(sym, "FLOAT2INT", call, out);
}

static bool cg_call_length(Symtable *sym, InstCall call, FILE *out) {
    OPRINTLN("# strlen");
    return cg_single_arg(sym, "STRLEN", call, out);
}

static bool cg_call_substring(Symtable *sym, InstCall call, FILE *out) {
    if (!call.dst.has_value) {
        return true;
    }

    InstSymb s = VEC_AT(&call.params, InstSymb, 0);
    InstSymb i = VEC_AT(&call.params, InstSymb, 1);
    InstSymb j = VEC_AT(&call.params, InstSymb, 2);

    InstSymb nil = {
        .type = IS_LITERAL,
        .literal = { .type = DT_NIL },
    };

    CHECKD(SymItem *, nil_l, sym_label(sym));
    CHECKD(SymItem *, end_l, sym_label(sym));
    CHECKD(SymItem *, loop_l, sym_label(sym));
    CHECKD(SymItem *, loop_end_l, sym_label(sym));

    OPRINTLN("# substring");

    OPRINTLN("CREATEFRAME");
    OPRINTLN("DEFVAR TF@!len");
    OPRINTLN("DEFVAR TF@!bin");

    OPRINT("GT TF@!bin ");
    CHECK(cg_write_symb(i, out));
    OPRINT(" ");
    CHECK(cg_write_symb(j, out));
    OPRINTLN("");
    OPRINTLN("JUMPIFEQ %s TF@!bin bool@true", nil_l->uname.str);

    OPRINT("LT TF@!bin ");
    CHECK(cg_write_symb(i, out));
    OPRINTLN(" int@0");
    OPRINTLN("JUMPIFEQ %s TF@!bin bool@true", nil_l->uname.str);

    OPRINT("STRLEN TF@!len ");
    CHECK(cg_write_symb(s, out));
    OPRINTLN("");

    OPRINT("GT TF@!bin ");
    CHECK(cg_write_symb(j, out));
    OPRINTLN(" TF@!len");
    OPRINTLN("JUMPIFEQ %s TF@!bin bool@true", nil_l->uname.str);

    OPRINTLN("DEFVAR TF@!res");
    OPRINTLN("DEFVAR TF@!chr");
    OPRINTLN("DEFVAR TF@!i");

    OPRINTLN("MOVE TF@!res string@");
    OPRINT("MOVE TF@!i ");
    CHECK(cg_write_symb(i, out));
    OPRINTLN("");

    OPRINT("JUMPIFEQ %s TF@!i ", loop_end_l->uname.str);
    CHECK(cg_write_symb(j, out));
    OPRINTLN("");

    OPRINTLN("LABEL %s", loop_l->uname.str);
    OPRINT("GETCHAR TF@!chr ");
    CHECK(cg_write_symb(s, out));
    OPRINTLN(" TF@!i");
    OPRINTLN("ADD TF@!i TF@!i int@1");
    OPRINTLN("CONCAT TF@!res TF@!res TF@!chr");
    OPRINT("JUMPIFNEQ %s TF@!i ", loop_l->uname.str);
    CHECK(cg_write_symb(j, out));
    OPRINTLN("");

    OPRINTLN("LABEL %s", loop_end_l->uname.str);
    if (call.dst.ident) {
        OPRINT("MOVE ");
        cg_write_ident(call.dst.ident, out);
        OPRINTLN(" TF@!res");
    } else {
        OPRINTLN("PUSHS TF@!res");
    }
    OPRINTLN("JUMP %s", end_l->uname.str);

    OPRINTLN("LABEL %s", nil_l->uname.str);
    CHECK(cg_get_value(nil, call.dst.ident, out));

    OPRINTLN("LABEL %s", end_l->uname.str);

    return true;
}

static bool cg_call_ord(Symtable *sym, InstCall call, FILE *out) {
    if (call.dst.has_value) {
        return true;
    }

    InstSymb c = VEC_AT(&call.params, InstSymb, 0);

    InstSymb zero = {
        .type = IS_LITERAL,
        .literal = { .type = DT_INT, .int_v = 0 },
    };

    CHECKD(SymItem *, end_l, sym_label(sym));
    CHECKD(SymItem *, empty_l, sym_label(sym));

    OPRINTLN("# ord");

    OPRINTLN("CREATEFRAME");
    OPRINTLN("DEFVAR TF@!len");

    OPRINT("STRLEN TF@!len ");
    CHECK(cg_write_symb(c, out));
    OPRINTLN("");
    OPRINTLN("JUMPIFEQ %s !len int@0", empty_l->uname.str);

    if (call.dst.ident) {
        OPRINT("STR2INT ");
        CHECK(cg_write_ident(call.dst.ident, out));
        OPRINT(" ");
        CHECK(cg_write_symb(c, out));
        OPRINTLN(" int@0");
    } else {
        OPRINTLN("DEFVAR TF@!chr");
        OPRINT("STR2INT TF@!chr ");
        CHECK(cg_write_symb(c, out));
        OPRINTLN(" int@0");
    }

    OPRINTLN("JUMP %s", end_l->uname.str);

    OPRINTLN("LABEL %s", empty_l->uname.str);
    CHECK(cg_get_value(zero, call.dst.ident, out));
    OPRINTLN("LABEL %s", end_l->uname.str);

    return true;
}

static bool cg_call_chr(Symtable *sym, InstCall call, FILE *out) {
    return cg_single_arg(sym, "INT2CHAR", call, out);
}

static bool cg_call(Symtable *sym, InstCall call, FILE *out) {
    OPRINTLN("# call");
    OPRINTLN("CREATEFRAME");
    VEC_FOR_EACH(&call.params, InstSymb, a) {
        InstSymb carg = *a.v;
        SymItem *darg = VEC_AT(&call.ident->func.params, FuncParam, a.i).ident;
        OPRINT("DEFVAR ");
        cg_write_tident(darg, true, out);
        OPRINTLN("");
        CHECK(cg_get_tvalue(carg, darg, true, out));
    }
    OPRINTLN("PUSHFRAME");
    OPRINTLN("CALL %s", call.ident->uname.str);

    if (!call.dst.has_value) {
        if (call.ident->func.return_type & DT_VOID) {
            return true;
        }
        OPRINTLN("CREATEFRAME");
        OPRINTLN("DEFVAR TF@!null");
        OPRINTLN("POPS TF@!null");
        return true;
    }

    return cg_get_value(STACK_SYMB, call.dst.ident, out);
}
