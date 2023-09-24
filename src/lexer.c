#include "lexer.h" // FILE, Lexer, NUL_STR, Token (and its values), NULL,
                   // sb_new, sb_free, true, false, bool

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
/// Reads operator or EOF
static bool skip_comment(Lexer *lex);
/// Reads next char, sets cur_char to the next char and returns the NEW char
static int next_chr(Lexer *lex);
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
    sb_clear(&lex->buffer);

    while (isspace(lex->cur_chr)) {
        next_chr(lex);
    }

    if (lex->cur_chr == '_' || isalpha(lex->cur_chr)) {
        lex->cur = read_ident(lex);
        return lex->cur;
    }

    if (isdigit(lex->cur_chr)) {
        lex->cur = read_num(lex);
        return lex->cur;
    }

    if (lex->cur_chr == '"') {
        lex->cur = read_str(lex);
        return lex->cur;
    }

    if (lex->cur_chr == '/') {
        if (next_chr(lex) == '/' || lex->cur_chr == '*') {
            if (!skip_comment(lex)) {
                return T_ERR;
            }
            return lex_next(lex);
        }
        return '/';
    }

    lex->cur = read_operator(lex);
    return lex->cur;
}

static Token read_ident(Lexer *lex) {
    TODO("lexer: read_ident");
}

static Token read_num(Lexer *lex) {
    TODO("lexer: read_num");
}

static Token read_str(Lexer *lex) {
    TODO("lexer: read_str");
}

static Token read_operator(Lexer *lex) {
    TODO("lexer: read_operator");
}

static bool skip_comment(Lexer *lex) {
    if (lex->cur_chr == '/') {
        while (next_chr(lex) != '\n' || lex->cur_chr != EOF)
            ;
        return true;
    }

    do {
        while (next_chr(lex) != '*') {
            if (lex->cur_chr == EOF) {
                lex->subtype = ERR_LEX;
                EPRINTF("Unexpected end of file, expected '*/'");
                return false;
            }
        }
    } while (next_chr(lex) != '/');

    return true;
}

static int next_chr(Lexer *lex) {
    return lex->cur_chr = fgetc(lex->in);
}

static int chr_next(Lexer *lex) {
    int cur = lex->cur_chr;
    next_chr(lex);
    return cur;
}
