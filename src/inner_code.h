#ifndef INNER_CODE_H_INCLUDED
#define INNER_CODE_H_INCLUDED

#include "symtable.h"

typedef enum {
    IT_MOVE,   // move, pops, pushs
    IT_DECL,   // defvar
    IT_CALL,   // call, createframe, pushframe, move, int2float, int2char
               // float2int, str2int, int2floats, float2ints, int2chars
               // str2ints, read, write, strlen, getchar, setchar, pops
               // pushs
    IT_RETURN, // return, popframe, createframe, pushs
    IT_ADD,    // add, adds, concat
    IT_MUL,    // mul, muls
    IT_DIV,    // div, idiv, divs, idivs
    IT_LT,     // lt, lts
    IT_GT,     // gt, gts
    IT_EQ,     // eq, eqs
    IT_ISNIL,  // type, eq
    IT_LTE,    // lt, lts, eq, eqs
    IT_GTE,    // gt, gts, eq, eqs
    IT_NOTNIL, // eq, type
    IT_LABEL,  // label
    IT_JUMP,   // jump
    IT_JIF,    // jumpifeq, jumpifeqs
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
    // NULL to push to stack
    SymItem *dst;
    InstSymb src;
} InstMove;

typedef struct {
    SymItem *var;
} InstDecl;

typedef struct {
    SymItem *dst;
    SymItem *ident;
    // type: InstSymb
    Vec params;
} InstCall;

typedef struct {
    size_t pop_count;
    bool has_value;
    InstSymb value;
} InstReturn;

typedef struct {
    // NULL to push to stack
    SymItem *dst;
    InstSymb first;
    InstSymb second;
} InstBinary;

typedef struct {
    // NULL to push to stack
    SymItem *dst;
    SymItem *src;
} InstIfNil;

typedef struct {
    InstSymb value;
} InstExit;

typedef struct {
    SymItem *ident;
} InstLabel;

typedef struct {
    SymItem *label;
    InstSymb first;
    InstSymb second;
} InstJmpBin;

typedef struct {
    SymItem *label;
    SymItem *src;
} InstJmpNil;

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
                             // IT_LTE, IT_GTE
        InstIfNil unary;     // IT_ISNIL, IT_NOTNIL
        InstExit exit;       // IT_EXIT
        InstLabel label;     // IT_LABEL, IT_JMP, IT_JIF
        InstJmpBin jmp_bin;  // IT_JEQ, IT_JNEQ, IT_JGT, IT_JLT, IT_JLTE,
                             // IT_JGTE
        InstJmpNil jmp_nil;  // IT_JISNIL, IT_JNONIL
        InstExit exit;       // IT_EXIT
    };
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


#endif // INNER_CODE_H_INCLUDED
