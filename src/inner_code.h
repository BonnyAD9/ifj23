#ifndef INNER_CODE_H_INCLUDED
#define INNER_CODE_H_INCLUDED

#include "symtable.h"

typedef enum {
    IT_MOVE,   // move
    IT_DECL,   // defvar
    IT_CALL,   // call, createframe, pushframe
    IT_RETURN,
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
        
    };
} InstSymb;

typedef struct {
    SymItem *dst;
    SymItem *src;
} InstMove;

typedef struct {
    SymItem *var;
} InstDecl;

typedef struct {
    SymItem *dst;
    union {
        SymItem *ident;

    };
} InstCallParam;

typedef struct {
    SymItem *dst;
    // type: InstCallParam
    Vec params;
} InstCall;

typedef struct {
    InstType type;
} Instruction;

#endif // INNER_CODE_H_INCLUDED
