#include "lexer.h" // FILE, Lexer, NUL_STR, Token (and its values), NULL,
                   // sb_new, sb_free

#include <ctype.h> // isspace

#include "utils.h" // TODO

/// Reads identifier, keyword or the '_' token
static Token read_ident(Lexer *lex);
/// Reads number (Int or Double)
static Token read_num(Lexer *lex);
/// Reads string (single or triple quoted)
static Token read_str(Lexer *lex);
/// Reads operator or EOF
static Token read_operator(Lexer *lex);
/// Reads next char, sets cur_char to the next char and returns the NEW char
static Token next_chr(Lexer *lex);
/// Reads next char, sets cur_char to the next char and returns the OLD char
static int chr_next(Lexer *lex);

Lexer lex_new(FILE *in) {
    return (Lexer) {
        .in = in,
        .cur_chr = ' ',
        // The first token can be anything, it is never readed
        .cur = T_ERR,
        .buffer = sb_new(),
        .str = NUL_STR,
    };
}

void lex_free(Lexer *lex) {
    if (!lex->in) {
        return;
    }

    fclose(lex->in);
    lex->in = NULL;
    sb_free(&lex->buffer);
    lex->str = NUL_STR;
}

Token lex_next(Lexer *lex) {
    while (isspace(lex->cur_chr)) {
        next_chr(lex);
    }

    if (lex->cur_chr == '_' || isalpha(lex->cur_chr)) {
        return read_ident(lex);
    }

    if (isdigit(lex->cur_chr)) {
        return read_num(lex);
    }

    if (lex->cur_chr == '"') {
        return read_str(lex);
    }

    return read_operator(lex);
}

static int read_ident(Lexer *lex) {
    TODO("lexer: read_ident");
}

static int read_num(Lexer *lex) {
    TODO("lexer: read_num");
}

static int read_str(Lexer *lex) {
    TODO("lexer: read_str");
}

static Token read_operator(Lexer *lex) {
    TODO("lexer: read_operator");
}

static int next_chr(Lexer *lex) {
    return lex->cur_chr = fgetc(lex->in);
}

static int chr_next(Lexer *lex) {
    int cur = lex->cur_chr;
    next_chr(lex);
    return cur;
}
