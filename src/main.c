#include <stdio.h>

#include "utils.h"
#include "lexer.h"
#include "stream.h"

int main(void) {
    FILE* file = fopen("test/testInput.txt", "r");
    if (!file)
        EPRINTF("Error opening input file");

    Stream in = stream_from_file(file);

    // Init mock lexer, let him read input and output parsed tokens
    Lexer lexer = lex_new(in);
    Token token = lex_next(&lexer);

    while (token != T_ERR && token != EOF) {
        fprintf(stdout, "Token %c|%d [%s] \n", token, token, lexer.str.str);
        token = lex_next(&lexer);
    }
    lex_free(&lexer);
}
