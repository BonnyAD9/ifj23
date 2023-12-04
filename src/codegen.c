#include "codegen.h"

#include <assert.h>

// helper macro, expects that there is FILE named out
#define OPRINTLN(fmt, ...) fprintf(out, fmt"\n", ##__VA_ARGS__)
// helper macro, expects that there is FILE named out
#define OPRINT(fmt, ...) fprintf(out, fmt,##__VA_ARGS__)

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

static bool cg_esc_string(String str, FILE *out);

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

static bool cg_call_read(Symtable *sym, const char *type, InstCall call, FILE *out);
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
    if (src.type == IS_IDENT && !src.ident) {
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

static bool cg_get_symb(Symtable *sym, InstSymb src, InstSymb *dst, FILE *out) {
    if (src.type != IS_IDENT || src.ident) {
        *dst = src;
        return true;
    }

    CHECKD(SymItem *, itm, todo_sym_tmp_var(sym, DT_NONE));

    OPTINT("POPS ");
    CHECK(cg_write_ident(itm, out));
    OPRINTLN("");

    *dst = INST_SYMB_ID(itm);
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

    OPRINTLN("RETURN");
    return true;
}

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

static bool cg_call_read(Symtable *sym, const char *type, InstCall call, FILE *out);
static bool cg_call_write(Symtable *sym, InstCall call, FILE *out);
static bool cg_call_int2double(Symtable *sym, InstCall call, FILE *out);
static bool cg_call_double2int(Symtable *sym, InstCall call, FILE *out);
static bool cg_call_length(Symtable *sym, InstCall call, FILE *out);
static bool cg_call_substring(Symtable *sym, InstCall call, FILE *out);
static bool cg_call_ord(Symtable *sym, InstCall call, FILE *out);
static bool cg_call_chr(Symtable *sym, InstCall call, FILE *out);
static bool cg_call(Symtable *sym, InstCall call, FILE *out);
