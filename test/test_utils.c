#include "test_utils.h"


#include <stdio.h>

void code_insert(char* test_code ) {
    FILE* test_file = fopen(TEST_FILE, "w");
    if (!test_file) {
        EPRINTF("Error opening testing input file\n");
    }

    fputs(test_code, test_file);
    //printf("TESTING WITH:\n%s\n", test_code);
    fclose(test_file);
}

void print_location( bool which_to_print ) {
    FILE* test_file = fopen(TEST_FILE, "r");
    if (!test_file) {
        EPRINTF("Error opening testing input file\n");
    }

    Stream in = stream_from_file(test_file, TEST_FILE);

    Lexer lexer = lex_new(in);
    Token token = lex_next(&lexer);

    while (token != T_ERR && token != EOF) {
        if(which_to_print) {
            printf("Token line [%zu]\n", lexer.token_start.line);
        }
        else {
            printf("Token column:[%zu]\n", lexer.token_start.column);
        }
        token = lex_next(&lexer);
    }
    lex_free(&lexer);
    fclose(test_file);
}

void print_token() {
    FILE* test_file = fopen(TEST_FILE, "r");
      if (!test_file) {
        EPRINTF("Error opening testing input file\n");
    }

    Stream in = stream_from_file(test_file, TEST_FILE);

    Lexer lexer = lex_new(in);
    Token token = lex_next(&lexer);

    while (token != T_ERR && token != EOF) {
        if (isprint(token)) {
            printf("Token: [%c | %d] [%s]\n", token, token, lexer.str.str);
        }
        else {
            printf("Token: [  | %d] [%s]\n", token, lexer.str.str);
        }
        token = lex_next(&lexer);
    }
    lex_free(&lexer);
    fclose(test_file);
}