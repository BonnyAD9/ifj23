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

    T_FUNC,

    T_IF,
    T_ELSE,
    T_WHILE,
    T_RETURN,

    /// the keyword 'let' or 'var'
    T_DECL,

    /// any of the types (Int, String, Double)
    T_TYPE,
} Token;

typedef enum {
    INT_TYPE,
    INT_TYPE_WITH_QST, // Int?
    DOUBLE_TYPE,
    DOUBLE_TYPE_WITH_QST, // Double?
    STRING_TYPE,
    STRING_TYPE_WITH_QST // String?
    // TODO add operators types here
} Subtype;

typedef struct {
    bool is_decimal_num; // if false => integer number
    bool with_exponent;
    unsigned long int integer_num;
    double decimal_num;
    long int exponent_num;
} Number;

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
    /// The parsed token data of number
    Number number;
    /// More detailed type of the token
    Subtype subtype;
} Lexer;

/// Crates new lexer, takes ownership of the file
Lexer lex_new(FILE *in);

/// Gets the next token, returns T_ERR on error
Token lex_next(Lexer *lex);

/// frees the lexer
void lex_free(Lexer *lex);
