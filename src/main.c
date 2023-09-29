#include <stdio.h>

#include "utils.h"
#include "lexer.h"

int main(void) {
    FILE* file = fopen("test/testInput.txt", "r");
    if (!file)
        EPRINTF("Error opening input file");

    // Init mock lexer, let him read input and output parsed tokens
    Lexer lexer = lex_new(file);
    Token token = lex_next(&lexer);

    while (token != T_ERR && token != EOF) {
        fprintf(stdout, "Token %c|%d [%s] \n", token, token, lexer.str.str);
        token = lex_next(&lexer);
    }
    lex_free(&lexer);
}
