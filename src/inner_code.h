#ifndef INNER_CODE_H_INCLUDED
#define INNER_CODE_H_INCLUDED

#include "symtable.h"
#include "ast.h"

#define INST_NONE_IDENT ((InstOptIdent) { .has_value = false })
#define INST_STACK_IDENT ((InstOptIdent) { \
    .has_value = true, \
    .ident = NULL, \
})
#define INST_IDENT(id) ((InstOptIdent) { \
    .has_value = true, \
    .ident = (id), \
})
#define INST_SYMB_ID(id) ((InstSymb) { \
    .type = IS_IDENT, \
    .ident = (id), \
})
#define INST_SYMB_LIT(lit) ((InstSymb) { \
    .type = IS_LITERAL, \
    .literal = lit, \
})

typedef enum {
    IT_MOVE,   // move, pops, pushs
    IT_DECL,   // defvar
    IT_CALL,   // call, createframe, pushframe, move, int2float, int2char
               // float2int, str2int, int2floats, float2ints, int2chars
               // str2ints, read, write, strlen, getchar, setchar, pops
               // pushs
    IT_RETURN, // return, popframe, createframe, pushs
    IT_SUB,    // sub, subs
    IT_ADD,    // add, adds
    IT_CONCAT, // concat
    IT_MUL,    // mul, muls
    IT_DIV,    // div, divs
    IT_IDIV,   // idiv, idivs
    IT_LT,     // lt, lts
    IT_GT,     // gt, gts
    IT_EQ,     // eq, eqs
    IT_ISNIL,  // type, eq
    IT_LTE,    // lt, lts, eq, eqs
    IT_GTE,    // gt, gts, eq, eqs
    IT_NEQ,    // eq, eqs, pushs
    IT_NOTNIL, // eq, type
    IT_LABEL,  // label
    IT_JUMP,   // jump
    IT_JIF,    // jumpifeq, jumpifeqs
    IT_JIFN,   // jumpifeq, jumpifeqs
    IT_JEQ,    // jumpifeq, jumpifeqs
    IT_JNEQ,   // jumpifneq, jumpifneqs
    IT_JLT,    // jumpifeq, jumpifeqs, lt, lts
    IT_JGT,    // jumpifeq, jumpifeqs, gt, gts
    IT_JISNIL, // jumpifeq, jumpifeqs, type, pops
    IT_JLTE,   // jumpifneq, jumpifneqs, gt
    IT_JGTE,   // jumpifneq, jumpifneqs, lt
    IT_JNONIL, // jumpifneq, jumpifneqs, type, pushs
    IT_EXIT,   // exit, pops
} InstType;

typedef struct {
    DataType type;
    union {
        int int_v;
        double double_v;
        String str;
    };
} InstLiteral;

typedef enum {
    IS_IDENT,
    IS_LITERAL,
} InstSymbType;

typedef struct {
    InstSymbType type;
    union {
        // When NULL take from stack
        SymItem *ident;
        InstLiteral literal;
    };
} InstSymb;

typedef struct {
    bool has_value;
    InstSymb value;
} InstOptSymb;

typedef struct {
    bool has_value;
    SymItem *ident;
} InstOptIdent;

// IT_MOVE
typedef struct {
    // NULL to push to stack
    SymItem *dst;
    InstSymb src;
} InstMove;

// IT_DECL
typedef struct {
    SymItem *var;
} InstDecl;

// IT_CALL
typedef struct {
    InstOptIdent dst;
    SymItem *ident;
    // type: InstSymb
    Vec params;
} InstCall;

// IT_RETURN
typedef struct {
    InstOptSymb value;
} InstReturn;

// IT_ADD, IT_MUL, IT_DIV, IT_LT, IT_GT, IT_EQ, IT_LTE, IT_GTE, IT_SUB, IT_NEQ,
// IT_IDIV, IT_CONCAT
typedef struct {
    // NULL to push to stack
    SymItem *dst;
    InstSymb first;
    InstSymb second;
} InstBinary;

// IT_ISNIL, IT_NOTNIL
typedef struct {
    // NULL to push to stack
    SymItem *dst;
    SymItem *src;
} InstIfNil;

// IT_LABEL, IT_JMP, IT_JIF, IT_JIFN
typedef struct {
    SymItem *ident;
} InstLabel;

// IT_JEQ, IT_JNEQ, IT_JGT, IT_JLT, IT_JLTE, IT_JGTE
typedef struct {
    SymItem *label;
    InstSymb first;
    InstSymb second;
} InstJmpBin;

// IT_JISNIL, IT_JNONIL
typedef struct {
    SymItem *label;
    SymItem *src;
} InstJmpNil;

// IT_EXIT
typedef struct {
    InstSymb value;
} InstExit;

typedef struct {
    InstType type;
    union {
        InstMove move;       // IT_MOVE
        InstDecl decl;       // IT_DECL
        InstCall call;       // IT_CALL
        InstReturn return_v; // IT_RETURN
        InstBinary binary;   // IT_ADD, IT_MUL, IT_DIV, IT_LT, IT_GT, IT_EQ
                             // IT_LTE, IT_GTE, IT_SUB, IT_NEQ, IT_IDIV,
                             // IT_CONCAT
        InstIfNil if_nil;    // IT_ISNIL, IT_NOTNIL
        InstLabel label;     // IT_LABEL, IT_JUMP, IT_JIF, IT_JIFN
        InstJmpBin jmp_bin;  // IT_JEQ, IT_JNEQ, IT_JGT, IT_JLT, IT_JLTE,
                             // IT_JGTE
        InstJmpNil jmp_nil;  // IT_JISNIL, IT_JNONIL
        InstExit exit;       // IT_EXIT
    };

    FilePos pos;
} Instruction;

typedef struct {
    SymItem *ident;
    // type: Instruction
    Vec code;
} FunctionCode;

typedef struct {
    // type: FunctionCode
    Vec functions;
    // type: Instruction
    Vec code;
} InnerCode;

bool ic_inner_code(Symtable *table, AstBlock *block, InnerCode *res);
void ic_free_code(InnerCode *code);
void ic_free_func_code(FunctionCode *code);
void ic_free_instruction(Instruction *inst);
void ic_free_symb(InstSymb *symb);
void ic_free_opt_symb(InstOptSymb *symb);

#endif // INNER_CODE_H_INCLUDED
