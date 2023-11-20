#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include <stdio.h> // EOF, FILE

#include "str.h" // String
#include "stream.h"

typedef enum {
    /// An error occured
    T_ERR = -2,
    /// End of input reached
    T_EOF = EOF,

    // char tokens here (e.g. ',' or '{')

    // other tokens:

    /// Any identifier
    T_IDENT = 256,
    /// Number/String/nil
    T_LITERAL,

    /// The '->' token
    T_RETURNS,
    /// The '==' token
    T_EQUALS,
    /// The '!=' token
    T_DIFFERS,
    /// The '<=' token
    T_LESS_OR_EQUAL,
    /// The '=>' token
    T_GREATER_OR_EQUAL,
    /// The '??' token
    T_DOUBLE_QUES,

    T_FUNC,

    T_IF,
    T_ELSE,
    T_WHILE,
    T_RETURN,

    /// The keyword 'let' or 'var'
    T_DECL,

    /// Any of the types (Int, String, Double)
    T_TYPE,

    // the following are never returned by lexer, but they are used in parser

    // e.g. in 'a = -a'
    T_UNARY_MINUS,
    // e.g. in 'a = +a'
    T_UNARY_PLUS,
    // '(' as expression, e.g. 'a = a * (a + b)' and not e.g. 'a = foo(a)'
    T_EXPR_PAREN,
} Token;

typedef enum {
    TD_LET,
    TD_VAR,
} DeclType;

typedef struct {
    Stream in;
    /// The last readed char
    int cur_chr;
    /// The last returned token, T_ERR if no lex_next was never called.
    Token cur;
    /// Internal buffer
    StringBuffer buffer;
    /// Position (in file) of the last token
    FilePos token_start;

    /// String of the last token (string is owned by lexer, clone it to store
    /// it elsewhere)
    String str;
    /// The parsed token data
    double d_num;
    /// The parsed token data
    int i_num;
    /// More detailed type of the token
    int subtype;
} Lexer;

/// Crates new lexer
Lexer lex_new(Stream in);

/// Gets the next token, returns T_ERR on error
Token lex_next(Lexer *lex);

/// frees the lexer - owner's responsibility!
void lex_free(Lexer *lex);

#endif // LEXER_H_INCLUDED
