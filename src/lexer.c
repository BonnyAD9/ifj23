#include "lexer.h" // FILE, Lexer, NUL_STR, Token (and its values), NULL,
                   // sb_new, sb_free

#include <ctype.h> // isspace, isalnum, isdigit
#include <string.h> // strchr, strlen, strtod, strtol,
#include <errno.h>

#include "utils.h" // TODO

// Avoid double usage of these chars for number parsing
bool plus_minus_used = false,
     exponent_used = false,
     decimal_sign_used = false;

/// Reads identifier, keyword or the '_' token
static Token read_ident(Lexer *lex);
/// Reads number (Int or Double)
static Token read_num(Lexer *lex);
/// Reads string (single or triple quoted)
static Token read_str(Lexer *lex);
/// Reads operator or EOF
static Token read_operator(Lexer *lex);
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
        .str = NUL_STR
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

// Lexer error
Token lex_error(Lexer *lex, char *msg) {
    EPRINTF(msg);
    lex->subtype = ERR_LEX;
    return T_ERR;
}

Token lex_next(Lexer *lex) {
    if (lex->buffer.str)
        sb_clear(&lex->buffer);

    // Skip white-spaces and unprintable chars
    while (isspace(lex->cur_chr) || !isprint(lex->cur_chr)) {
        if (lex->cur_chr == EOF)
            return T_EOF;
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

    if (str_eq(lex->str, STR("Double")))
        return T_TYPE;
    else if (str_eq(lex->str, STR("else")))
        return T_ELSE;
    else if (str_eq(lex->str, STR("func")))
        return T_FUNC;
    else if (str_eq(lex->str, STR("if")))
        return T_IF;
    else if (str_eq(lex->str, STR("Int")))
        return T_TYPE;
    else if (str_eq(lex->str, STR("let")))
        return T_DECL;
    else if (str_eq(lex->str, STR("nil")))
        return T_NIL;
    else if (str_eq(lex->str, STR("return")))
        return T_RETURN;
    else if (str_eq(lex->str, STR("String")))
        return T_TYPE;
    else if (str_eq(lex->str, STR("var")))
        return T_DECL;
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

    // Skip noprintable chars
    if (!isprint(lex->cur_chr))
        return false;

    // Non-numerical value
    switch (lex->cur_chr) {
        case '=':
        case '>':
        case '<':
        case '!':
        case ',':
        case ' ':
        case '(':
        case ')':
        case '*':
        case '/':
            if (isalpha(prev))
                // f.e. "2E " is error
                return lex_error(lex, "Invalid number format \n");
            // f.e. "5<="" isn't error
            return false;
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
            return lex_error(lex, "Invalid number format \n");
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
            return T_DLIT;
    }
    else {
        // Integer number
        // Store integer number of base 10
        lex->i_num = strtol(lex->str.str, &endval, 10);
        if (endval == lex->str.str || errno != 0)
            return lex_error(lex, "Error while converting number \n");
        return T_ILIT;
    }
}

static Token read_str(Lexer *lex) {
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

    return T_SLIT;
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

static int next_chr(Lexer *lex) {
    if (lex && lex->in)
        return lex->cur_chr = fgetc(lex->in);
    return '\0';
}

static int chr_next(Lexer *lex) {
    int cur = lex->cur_chr;
    next_chr(lex);
    return cur;
}
