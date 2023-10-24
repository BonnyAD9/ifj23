#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "lexer.h" // Lexer, Token

typedef int TodoTree; // this will be changed when AstTree is finished

typedef struct {
    Lexer *lex;
    Token cur;
    int error;
} Parser;

Parser parser_new(Lexer *lex);

bool parser_parse(Parser *par);

void parser_free(Parser *p);

#endif // PARSER_H_INCLUDED
