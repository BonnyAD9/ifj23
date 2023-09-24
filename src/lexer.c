#include "lexer.h" // FILE, Lexer, NUL_STR, Token (and its values), NULL,
                   // sb_new, sb_free

#include <ctype.h> // isspace, isalnum, isdigit
#include <string.h> // strchr, strlen, strtod, strtol,

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
        .str = NUL_STR,
        .number = {
            .is_decimal_num = false,
            .with_exponent = false
            // Rest values are not needed now
        }
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

    lex->cur = read_operator(lex);
    return lex->cur;
}

bool is_valid_ident(String string) {
    // Error case when ident is only "_"
    if (str_eq_const_str(string, "_") == true)
        return false;

    size_t index;
    for (index = string.len; index >= 0; --index) {
        if (isalnum(string.str[index]) == 0 || string.str[index] == '_')
            break;
    }
    // If we iterate to the end, it's valid ident
    return index == 0;
}

bool is_valid_number(String string) {
    size_t index;
    for (index = string.len; index >= 0; --index) {
        char temp = string.str[index];
        if (isdigit(temp) == 0) {
            // Non digit char
            // Check for allowed non-digit chars
            if (temp == '.' || temp == 'e' || temp == 'E' || temp == '+' || temp == '-')
                continue;
            else
                break;
        }
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

    if (str_eq_const_str(lex->str, "Double") == true || str_eq_const_str(lex->str, "Double?") == true) {
        // Check if last char is "?"
        lex->subtype = ((lex->str.str[lex->str.len] == '?') ?
            DOUBLE_TYPE_WITH_QST : DOUBLE_TYPE);
        return T_TYPE;
    }
    else if (str_eq_const_str(lex->str, "else") == true)
        return T_ELSE;
    else if (str_eq_const_str(lex->str, "func") == true)
        return T_FUNC;
    else if (str_eq_const_str(lex->str, "if") == true)
        return T_IF;
    else if (str_eq_const_str(lex->str, "Int") == true || str_eq_const_str(lex->str, "Int?") == true) {
        // Check if last char is "?"
        lex->subtype = ((lex->str.str[lex->str.len] == '?') ?
            INT_TYPE_WITH_QST : INT_TYPE);
        return T_TYPE;
    }
    else if (str_eq_const_str(lex->str, "let") == true)
        return T_DECL;
    else if (str_eq_const_str(lex->str, "nil") == true)
        return T_NIL;
    else if (str_eq_const_str(lex->str, "return") == true)
        return T_RETURN;
    else if (str_eq_const_str(lex->str, "String") == true || str_eq_const_str(lex->str, "String?") == true) {
        // Check if last char is "?"
        lex->subtype = ((lex->str.str[lex->str.len] == '?') ?
            STRING_TYPE_WITH_QST : STRING_TYPE);
        return T_TYPE;
    }
    else if (str_eq_const_str(lex->str, "var") == true)
        return T_DECL;
    else if (str_eq_const_str(lex->str, "while") == true)
        return T_WHILE;
    else {
        // Check if ident contains only alpha-numerical values
        if (is_valid_ident(lex->str))
            return T_IDENT;
        lex->str = NUL_STR;
        return T_ERR;
    }
}

static Token read_num(Lexer *lex) {
    // Load whole number into buffer
    while (isspace(lex->cur_chr) == 0) {
        sb_push(&lex->buffer, lex->cur_chr);
        next_chr(lex);
    }
    // Store token data
    lex->str = sb_get(&lex->buffer);
    // Check for valid chars in given number
    if (is_valid_number(lex->str)) {
        // Check for exponent
        char *ret = strchr(lex->str.str, 'e');

        if (!ret) // Try 'E'
            ret = strchr(lex->str.str, 'E');
        if (ret) {
            // Exponent found
            if (strlen(ret) > 1 && // Exponent is not at the of number
                ret != lex->str.str) { // Exponent is not at the begging of number
                lex->number.with_exponent = true;
                // Use value after 'e'/'E'
                lex->number.exponent_num = strtod((++ret), NULL);
            }
            else
                return T_ERR;
        }
        else
            lex->number.with_exponent = false;

        ret = strchr(lex->str.str, '.');
        // Decimal number
        if (ret) {
            if (strlen(ret) > 1 && // '.' is not at the end of number
                ret != lex->str.str) { // '.' is not at the begging of number, could be also done via strlen() comparison
                    lex->number.is_decimal_num = true;
                    // Store decimal number
                    lex->number.decimal_num = strtod(lex->str.str, NULL);
            }
            else
                return T_ERR;
        }
        // Integer number
        else {
            lex->number.is_decimal_num = false;
            // Store integer number of base 10
            lex->number.integer_num = strtol(lex->str.str, NULL, 10);
        }
    }
    return ((lex->number.is_decimal_num) ? T_DLIT : T_ILIT);
}

static Token read_str(Lexer *lex) {
    char c;
    // Store initial '"'
    sb_push(&lex->buffer, lex->cur_chr);
    next_chr(lex);

    // Load whole string into buffer
    while (lex->cur_chr != '"' && lex->cur_chr != '\n') {
        sb_push(&lex->buffer, lex->cur_chr);
        // Store previous char
        c = chr_next(lex);
    }

    // String is not properly ended
    if (c != '"')
        return T_ERR;

    TODO("lexer: read_str escape sequencion");

    // Store string
    lex->str = sb_get(&lex->buffer);
    return T_SLIT;
}

static Token read_operator(Lexer *lex) {
    TODO("lexer: read_operator");
    TODO("lexer: add operators enums");
}

static int next_chr(Lexer *lex) {
    return lex->cur_chr = fgetc(lex->in);
}

static int chr_next(Lexer *lex) {
    int cur = lex->cur_chr;
    next_chr(lex);
    return cur;
}
