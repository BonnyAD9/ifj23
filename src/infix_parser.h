/**
 * IFJ23
 *
 * xstigl00 Jakub Antonín Štigler
 */

#ifndef INFIX_PARSER_H_INCLUDED
#define INFIX_PARSER_H_INCLUDED

#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "str.h"

// struct that holds all info about token
typedef struct {
    Token type;
    int subtype;
    union {
        int int_v;
        double double_v;
        String str;
        SymItem *ident;
    };
} FullToken;

AstExpr *parse_infix(Parser *par);

#endif // INFIX_PARSER_H_INCLUDED
