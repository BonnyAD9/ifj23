#include <stdio.h> // EOF, FILE

#include "str.h" // String

typedef enum {
    /// An error occured
    T_ERR = -2,
    /// End of input reached
    T_EOF = EOF,

    // char tokens here (e.g. ',' or '{')

    // other tokens:

    /// Any identifier
    T_IDENT = 256,
    /// The 'nil' keyword
    T_NIL,
    /// String literal
    T_SLIT,
    /// Int literal
    T_ILIT,
    /// Double literal
    T_DLIT,

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
} Token;

typedef struct {
    FILE *in;
    /// The last readed char
    int cur_chr;
    /// The last returned token, T_ERR if no lex_next was never called.
    Token cur;
    /// Internal buffer
    StringBuffer buffer;

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

/// Crates new lexer, takes ownership of the file
Lexer lex_new(FILE *in);

/// Gets the next token, returns T_ERR on error
Token lex_next(Lexer *lex);

/// frees the lexer - owner's responsibility!
void lex_free(Lexer *lex);