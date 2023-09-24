#include "lexer.h" // FILE, Lexer, NUL_STR, Token (and its values), NULL,
                   // sb_new, sb_free

#include <ctype.h> // isspace, isalnum, isdigit
#include <string.h> // strchr, strlen, strtod, strtol,
#include <errno.h>

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

Token lex_next(Lexer *lex) {
    sb_clear(&lex->buffer);

    // Skip white-spaces and unprintable chars
    while (isspace(lex->cur_chr) || !isprint(lex->cur_chr)) {
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

bool is_valid_ident(String string) {
    // Error case when ident is only "_"
    if (str_eq(string, STR("_")) == true)
        return false;

    size_t index;
    for (index = string.len; index >= 0; --index) {
        if (isalnum(string.str[index]) == 0 || string.str[index] == '_')
            break;
    }
    // If we iterate to the end, it's valid ident
    return index == 0;
}

static Token read_ident(Lexer *lex) {
    // Load whole ident into buffer
    while (isspace(lex->cur_chr) == 0) {
        sb_push(&lex->buffer, lex->cur_chr);
        next_chr(lex);
    }

    // Store token data
    lex->str = sb_get(&lex->buffer);

    if (str_eq(lex->str, STR("Double")) || str_eq(lex->str, STR("Double?"))) {
        // Check if last char is "?"
        lex->subtype = ((lex->str.str[lex->str.len] == '?') ?
            DOUBLE_TYPE_WITH_QST : DOUBLE_TYPE);
        return T_TYPE;
    }
    else if (str_eq(lex->str, STR("else")))
        return T_ELSE;
    else if (str_eq(lex->str, STR("func")))
        return T_FUNC;
    else if (str_eq(lex->str, STR("if")))
        return T_IF;
    else if (str_eq(lex->str, STR("Int")) || str_eq(lex->str, STR("Int?"))) {
        // Check if last char is "?"
        lex->subtype = ((lex->str.str[lex->str.len] == '?') ?
            INT_TYPE_WITH_QST : INT_TYPE);
        return T_TYPE;
    }
    else if (str_eq(lex->str, STR("let")))
        return T_DECL;
    else if (str_eq(lex->str, STR("nil")))
        return T_NIL;
    else if (str_eq(lex->str, STR("return")))
        return T_RETURN;
    else if (str_eq(lex->str, STR("String")) || str_eq(lex->str, STR("String?"))) {
        // Check if last char is "?"
        lex->subtype = ((lex->str.str[lex->str.len] == '?') ?
            STRING_TYPE_WITH_QST : STRING_TYPE);
        return T_TYPE;
    }
    else if (str_eq(lex->str, STR("var")))
        return T_DECL;
    else if (str_eq(lex->str, STR("while")))
        return T_WHILE;
    else {
        // Check if ident contains only alpha-numerical values
        if (is_valid_ident(lex->str))
            return T_IDENT;
        lex->str = NUL_STR;
        return T_ERR;
    }
}

bool is_valid_number(char current, char prev) {
    if (!isdigit(current)) {
        // Non-digit char
        // If initial char is not number -> false
        if (prev != '\0') {
            if (isdigit(prev)) {
                // Previous char was digit
                if (current == '.' || current == 'e' || current == 'E')
                    return true;
            }
            else {
                // Previous char was non-digit
                if (current == 'e' || current == 'E' || current == '+' || current == '-')
                    return true;
            }
        }
        return false;
    }
    return true;
}

static Token read_num(Lexer *lex) {
    char c = '\0';
    // Load whole number into buffer
    while (is_valid_number(lex->cur_chr, c)) {
        sb_push(&lex->buffer, lex->cur_chr);
        // Store previous char
        c = chr_next(lex);
    }
    // Store token data
    lex->str = sb_get(&lex->buffer);

    char *ret = strchr(lex->str.str, '.');
    char *endval;
    // Reset before use
    errno = 0;

    // Decimal number
    if (ret) {
        if (strlen(ret) > 1 && // '.' is not at the end of number
            ret != lex->str.str) { // '.' is not at the begging of number, could be also done via strlen() comparison
                // Store decimal number
                lex->d_num = strtod(lex->str.str, &endval);
                // Error if no value was parsed or errno occured
                if (endval == lex->str.str || errno != 0)
                    return T_ERR;
                return T_DLIT;
        }
        else
            return T_ERR;
    }
    // Integer number
    else {
        // Store integer number of base 10
        lex->i_num = strtol(lex->str.str, &endval, 10);
        if (endval == lex->str.str || errno != 0)
            return T_ERR;
        return T_ILIT;
    }
}

static Token read_str(Lexer *lex) {
    TODO("Triple-uvozovky case");

    // Skip initial '"'
    next_chr(lex);

    // Load whole string into buffer
    while (lex->cur_chr != '\n' && lex->cur_chr != EOF) {
        if (lex->cur_chr == '\\') {
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
                            sb_push(&temp, lex->cur_chr);
                            if (!isxdigit(lex->cur_chr)) {
                                // Unexpected char provided, f.e. \u{65K
                                // Current approach is to push all of this to string, don't remove it
                                char *user_str = "\\u{";
                                // Link together with buffer values
                                strcat(user_str, temp.str);
                                // Push it to lexor's buffer
                                sb_push_str(&lex->buffer, user_str);
                                sb_free(&temp);
                                break;
                            }
                            next_chr(lex);
                        }

                        if (temp.len != 0) {
                            // Evaluate what's stored inside and push it to buffer
                            sb_push(&lex->buffer, strtol(temp.str, NULL, 16));
                            sb_free(&temp);
                        }
                    }
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
        else {
            sb_push(&lex->buffer, lex->cur_chr);
            next_chr(lex);
            if (lex->cur_chr == '"')
                break;
        }
    }

    // String is not properly ended
    if (lex->cur_chr != '"')
        return T_ERR;

    // Store string
    lex->str = sb_get(&lex->buffer);
    return T_SLIT;
}

static Token read_operator(Lexer *lex) {
    // Load whole operator into buffer
    // Operator should NOT be number/alpha
    while (!isalpha(lex->cur_chr) && !isdigit(lex->cur_chr)) {
        sb_push(&lex->buffer, lex->cur_chr);
        next_chr(lex);
    }

    // Store string
    lex->str = sb_get(&lex->buffer);
    return T_OPER;
}

static int next_chr(Lexer *lex) {
    return lex->cur_chr = fgetc(lex->in);
}

static int chr_next(Lexer *lex) {
    int cur = lex->cur_chr;
    next_chr(lex);
    return cur;
}
