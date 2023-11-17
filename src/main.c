#include <stdio.h>
#include <ctype.h>

#define DEBUG_FILE "test/testInput.swift"

#include "utils.h"
#include "lexer.h"
#include "stream.h"
#include "symtable.h"
#include "vec.h"
#include "ast.h"

int main(void) {
    FILE* file = fopen(DEBUG_FILE, "r");
    if (!file)
        EPRINTF("Error opening input file\n");

    Stream in = stream_from_file(file, DEBUG_FILE);

    // Init mock lexer, let him read input and output parsed tokens
    Lexer lexer = lex_new(in);
    Token token = lex_next(&lexer);

    while (token != T_ERR && token != EOF) {
        if (isprint(token)) {
            printf(
                DEBUG_FILE ":%zu:%zu: Token %c|%d [%s]\n",
                lexer.token_start.line,
                lexer.token_start.column,
                token,
                token,
                lexer.buffer.str
            );
        }
        else {
            printf(
                DEBUG_FILE ":%zu:%zu: Token |%d [%s]\n",
                lexer.token_start.line,
                lexer.token_start.column,
                token,
                lexer.buffer.str
            );
        }
        token = lex_next(&lexer);
    }
    fclose(file);
    lex_free(&lexer);
    printf("----------------------------------------\n");
}
