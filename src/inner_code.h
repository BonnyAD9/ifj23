#ifndef INNER_CODE_H_INCLUDED
#define INNER_CODE_H_INCLUDED

#include "symtable.h"

typedef enum {
    IT_MOVE,   // move, pops, pushs
    IT_DECL,   // defvar
    IT_CALL,   // call, createframe, pushframe, move
    IT_RETURN, // return, popframe, createframe
    IT_ADD,    // adds
    IT_MUL,    // muls
    IT_DIV,    // divs, idivs
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
        InstLiteral *literal;
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
    // type: InstSymb
    Vec params;
} InstCall;

typedef struct {
    size_t pop_count;
} InstReturn;

typedef struct {
    // NULL to push to stack
    SymItem *dst;
    InstSymb *first;
    InstSymb *second;
} InstBinary;

typedef struct {
    InstType type;
    union {
        InstMove move;       // IS_MOVE
        InstDecl decl;       // IS_DECL
        InstCall call;       // IS_CALL
        InstReturn return_v; // IS_RETURN
        InstBinary binary;   // IS_ADD, IS_MUL, IS_DIV
    };
} Instruction;

#endif // INNER_CODE_H_INCLUDED
