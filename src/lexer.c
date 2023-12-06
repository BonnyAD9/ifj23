/**
 * IFJ23
 *
 * xdanie14 Tomáš Daniel
 * xsleza23 Martin Slezák
 * xstigl00 Jakub Antonín Štigler
 */

#include "lexer.h" // FILE, Lexer, NUL_STR, Token (and its values), NULL,
                   // sb_new, sb_free, Stream, NUL_STREAM, stream_is_invalid

#include <ctype.h> // isspace, isalnum, isdigit
#include <string.h> // strchr, strlen, strtod, strtol,
#include <errno.h>

#include "utils.h" // DEBUG_FILE
#include "enums.h" // DataType::*
#include "debug_tools.h"

// Avoid double usage of these chars for number parsing
bool plus_minus_used   = false,
     exponent_used     = false,
     decimal_sign_used = false;

/// Reads identifier, keyword or the '_' token
static Token read_ident(Lexer *lex);
/// Reads number (Int or Double)
static Token read_num(Lexer *lex);
/// Reads string (single or triple quoted)
static Token read_str(Lexer *lex);
/// Reads operator
static Token read_operator(Lexer *lex);
/// Skips line or block comment, returns false if error occured
static bool skip_comment(Lexer *lex);
/// Reads next char, sets cur_char to the next char and returns the NEW char
static int next_chr(Lexer *lex);
/// Reads next char, sets cur_char to the next char and returns the OLD char
static int chr_next(Lexer *lex);

Lexer lex_new(Stream in) {
    return (Lexer) {
        .in = in,
        .cur_chr = ' ',
        // The first token can be anything, it is never readed
        .cur = T_ERR,
        .buffer = sb_new(),
        .str = NUL_STR
    };
}

void lex_free(Lexer *lex) {
    if (stream_is_invalid(&lex->in)) {
        return;
    }

    lex->in = NUL_STREAM;
    sb_free(&lex->buffer);
    lex->str = NUL_STR;
}

// Lexer error
Token lex_error(Lexer *lex, char *msg) {
    set_err_code(ERR_LEX);
    EPRINTF(
        DEBUG_FILE ":%zu:%zu: error: %s\n",
        lex->token_start.line,
        lex->token_start.column,
        msg
    );

    lex->subtype = ERR_LEX;
    return T_ERR;
}

Token lex_next(Lexer *lex) {
    sb_clear(&lex->buffer);

    // Skip white-spaces
    while (isspace(lex->cur_chr))
        next_chr(lex);

    lex->token_start = stream_get_pos(&lex->in);

    // End if EOF reached
    if (lex->cur_chr == EOF)
        return T_EOF;

    // Throw error on nonprintable character
    if (!isprint(lex->cur_chr))
        return lex_error(lex, "Nonprintable character in input file \n");
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

    int next_chr = stream_peak(&lex->in);
    if (lex->cur_chr == '/' && (next_chr == '/' || next_chr == '*')) {
        if (!skip_comment(lex)) {
            return T_ERR;
        }
        return lex_next(lex);
    }

    lex->cur = read_operator(lex);
    return lex->cur;
}

static Token read_ident(Lexer *lex) {
    // Load whole ident into buffer
    while (isalnum(lex->cur_chr) || lex->cur_chr == '_') {
        sb_push(&lex->buffer, lex->cur_chr);
        next_chr(lex);
    }

    // Store token data
    lex->str = sb_get(&lex->buffer);

    if (str_eq(lex->str, STR("Double"))) {
        lex->subtype = DT_DOUBLE;
        return T_TYPE;
    }
    else if (str_eq(lex->str, STR("else")))
        return T_ELSE;
    else if (str_eq(lex->str, STR("func")))
        return T_FUNC;
    else if (str_eq(lex->str, STR("if")))
        return T_IF;
    else if (str_eq(lex->str, STR("Int"))) {
        lex->subtype = DT_INT;
        return T_TYPE;
    }
    else if (str_eq(lex->str, STR("let"))) {
        lex->subtype = TD_LET;
        return T_DECL;
    }
    else if (str_eq(lex->str, STR("var"))) {
        lex->subtype = TD_VAR;
        return T_DECL;
    }
    else if (str_eq(lex->str, STR("nil"))){
        lex->subtype = DT_NIL;
        return T_LITERAL;
    }
    else if (str_eq(lex->str, STR("return")))
        return T_RETURN;
    else if (str_eq(lex->str, STR("String"))) {
        lex->subtype = DT_STRING;
        return T_TYPE;
    }
    else if (str_eq(lex->str, STR("while")))
        return T_WHILE;
    else if (str_eq(lex->str, STR("_")))
        return '_';
    // Generic ident
    return T_IDENT;
}

int is_valid_number(Lexer* lex, char prev) {
    if (isdigit(lex->cur_chr))
        return true;

    // Non-numerical value
    switch (lex->cur_chr) {
        case 'e':
        case 'E':
            if (isdigit(prev) && !exponent_used) {
                exponent_used = true;
                return true;
            }
            return false;
        case '+':
        case '-':
            if ((prev == 'e' || prev == 'E') && !plus_minus_used) {
                plus_minus_used = true;
                return true;
            }
            return false;
        case '.':
            if (isdigit(prev) && !exponent_used) {
                if (!decimal_sign_used) {
                    decimal_sign_used = true;
                    return true;
                }
                return false;
            }
        default:
            if (isalpha(lex->cur_chr) || lex->cur_chr == '_')
                return lex_error(lex, "Invalid number format \n");
            return false;
    }
}

static Token read_num(Lexer *lex) {
    int val;
    char c = '\0';

    // Reset to default values
    plus_minus_used = false;
    exponent_used = false;
    decimal_sign_used = false;

    // Load whole number into buffer
    while ((val = is_valid_number(lex, c))) {
        // Error in number format
        if (val == T_ERR)
            return T_ERR;

        sb_push(&lex->buffer, lex->cur_chr);
        // Store previous char
        c = chr_next(lex);
    }
    // Store token data
    lex->str = sb_get(&lex->buffer);

    char *endval;
    // Reset before use
    errno = 0;

    // Decimal number
    if (decimal_sign_used || exponent_used) {
            // Store decimal number
            lex->d_num = strtod(lex->str.str, &endval);
            // Error if no value was parsed or errno occured
            if (endval == lex->str.str || errno != 0)
                return lex_error(lex, "Error while converting number \n");
            lex->subtype = DT_DOUBLE;
            return T_LITERAL;
    }
    else {
        // Integer number
        // Store integer number of base 10
        lex->i_num = strtol(lex->str.str, &endval, 10);
        if (endval == lex->str.str || errno != 0)
            return lex_error(lex, "Error while converting number \n");
        lex->subtype = DT_INT;
        return T_LITERAL;
    }
}

static Token read_triple_str(Lexer *lex) {
    StringBuffer str = sb_new();

    // Skips new line after triple quotes
    if (next_chr(lex) != '\n') {
        sb_free(&str);
        return lex_error(
            lex,
            "no new line after opening triple quotes"
        );
    }
    next_chr(lex);

    int quote_cnt = 0;
    // Reads string to buffer
    while (true) {
        if (lex->cur_chr == '"') {
            ++quote_cnt;
            // End of multiline string
            if (quote_cnt == 3)
                break;
        } else {
            // Previous quotes
            for (; quote_cnt > 0; --quote_cnt)
                sb_push(&str, '"');

            sb_push(&str, lex->cur_chr);
        }
        next_chr(lex);

        // Multiline string not ended properly
        if (lex->cur_chr == EOF) {
            sb_free(&str);
            return lex_error(lex, "multiline string not closed");
        }
    }

    next_chr(lex); // skip the last '"'

    // Checks indentation count
    int indent = str.len - 1;
    for (; str.str[indent] != '\n'; --indent) {
        if (str.str[indent] != ' ') {
            sb_free(&str);
            return lex_error(
                lex,
                "no new line before closing triple quotes"
            );
        }
    }
    indent = str.len - indent - 1;

    // Removes indentation
    bool new_line = true;
    for (int i = 0; i < str.len - indent; ++i) {
        if (new_line) {
            for (int j = 0; j < indent; ++j) {
                if (str.str[i++] != ' ') {
                    sb_free(&str);
                    return lex_error(
                        lex,
                        "invalid indenation in mutliline string"
                    );
                }
            }
            --i;
            new_line = false;
        } else if (str.str[i] == '\n')
            new_line = true;
        else {
            sb_push(&lex->buffer, str.str[i]);
        }
    }
    sb_free(&str);

    // Store string
    lex->str = sb_get(&lex->buffer);

    lex->subtype = DT_STRING;
    return T_LITERAL;
}

static Token read_str(Lexer *lex) {
    // Triple double quotes case
    if (stream_peak(&lex->in) == '"') {
        next_chr(lex);
        if (next_chr(lex) == '"')
            return read_triple_str(lex);

        // Empty string
        lex->str = STR("");
        lex->subtype = DT_STRING;
        return T_LITERAL;
    }

    // Load whole string into buffer
    while (lex->cur_chr != '\n' && lex->cur_chr != EOF) {
        next_chr(lex);
        // case of "aa\{56}"
        if (lex->cur_chr != '\\') {
            if (lex->cur_chr == '"')
                break;
            sb_push(&lex->buffer, lex->cur_chr);
            continue;
        }
        // '\' char
        next_chr(lex);
        switch (lex->cur_chr) {
            case 'u':
                next_chr(lex);
                if (lex->cur_chr == '{') {
                    StringBuffer temp = sb_new();
                    next_chr(lex);
                    // Read 8 hexa values at max
                    for (; lex->cur_chr != '}' && temp.len < 8; ) {
                        if (!isxdigit(lex->cur_chr)) {
                            // Unexpected char provided, f.e. \u{65K
                            // Current approach is to push all of this to string, don't remove it
                            sb_push_str(&lex->buffer, "\\u{");
                            sb_push_str(&lex->buffer, temp.str);
                            sb_push(&lex->buffer, lex->cur_chr);
                            sb_free(&temp);
                            break;
                        }
                        sb_push(&temp, lex->cur_chr);
                        next_chr(lex);
                    }

                    // Temp is non-empty
                    if (temp.len)
                        // Evaluate what's stored inside and push it to buffer (limit range 0-255)
                        sb_push(&lex->buffer, (strtol(temp.str, NULL, 16) % 256));
                    else if (temp.str)
                        // Push remaining
                        sb_push_str(&lex->buffer, "\\u{}");
                    sb_free(&temp);
                }
                // Push remaining
                else
                    sb_push_str(&lex->buffer, "\\u");
                break;
            case '"':
                sb_push(&lex->buffer, '"');
                break;
            case 'n':
                sb_push(&lex->buffer, '\n');
                break;
            case 'r':
                sb_push(&lex->buffer, '\r');
                break;
            case 't':
                sb_push(&lex->buffer, '\t');
                break;
            case '\\':
                sb_push(&lex->buffer, '\\');
                break;
            default:
                // If not matched just push it to buffer as it is
                sb_push(&lex->buffer, '\\');
                sb_push(&lex->buffer, lex->cur_chr);
                break;
        }
    }

    // String is not properly ended
    if (lex->cur_chr != '"')
        return lex_error(lex, "String is missing ending pair symbol \" \n");

    // Skip '"' from being processed next time by lexer
    next_chr(lex);

    // Store string
    lex->str = sb_get(&lex->buffer);

    lex->subtype = DT_STRING;
    return T_LITERAL;
}

Token ret_operator(Lexer *lex, char symbol, Token ret_val) {
    char old = chr_next(lex);
    if (lex->cur_chr == symbol) {
        next_chr(lex);
        return ret_val;
    }

    return old;
}

static Token read_operator(Lexer *lex) {
    switch (lex->cur_chr) {
        case '+':
        case '*':
        case '/':
        case '(':
        case ')':
        case '{':
        case '}':
        case ':':
        case ',':
            return chr_next(lex);
        case '=':
            return ret_operator(lex, '=', T_EQUALS);
        case '!':
            return ret_operator(lex, '=', T_DIFFERS);
        case '<':
            return ret_operator(lex, '=', T_LESS_OR_EQUAL);
        case '>':
            return ret_operator(lex, '=', T_GREATER_OR_EQUAL);
        case '-':
            return ret_operator(lex, '>', T_RETURNS);
        case '?':
            return ret_operator(lex, '?', T_DOUBLE_QUES);
        default:
            return lex_error(lex, "Invalid operator provided \n");
    }
}

static bool skip_comment(Lexer *lex) {
    next_chr(lex); // skip and move to comment content

    if (lex->cur_chr == '/') {
        while (next_chr(lex) != '\n' && lex->cur_chr != EOF)
            ;
        return true;
    }

    // Initially we already loaded '/*' once
    unsigned int comment_parenthese_count = 1;

    do {
        char chr = next_chr(lex);
        while (chr != '*' && chr != '/') {
            if (lex->cur_chr == EOF) {
                lex->subtype = ERR_LEX;
                lex_error(lex, "Unexpected end of file, expected '*/'");
                return false;
            }
            chr = next_chr(lex);
        }
        // '*' or '/' at input
        next_chr(lex);

        if (chr == '/' && lex->cur_chr == '*') // -> '/*'
            ++comment_parenthese_count;
        else if (chr == '*' && lex->cur_chr == '/') // -> '*/'
            --comment_parenthese_count;
    } while (comment_parenthese_count);

    next_chr(lex); // skip the last '/'

    return true;
}

static int next_chr(Lexer *lex) {
    if (lex)
        return lex->cur_chr = stream_get(&lex->in);
    return EOF;
}

static int chr_next(Lexer *lex) {
    int cur = lex->cur_chr;
    next_chr(lex);
    return cur;
}
