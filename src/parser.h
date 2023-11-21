#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "lexer.h" // Lexer, Token
#include "vec.h"
#include "symtable.h"
#include "ast.h"

typedef struct {
    Lexer *lex;
    Symtable *table;
    Token cur;
    int error;
} Parser;

Parser parser_new(Lexer *lex, Symtable *table);

AstBlock *parser_parse(Parser *par);

void parser_free(Parser *p);

#endif // PARSER_H_INCLUDED
