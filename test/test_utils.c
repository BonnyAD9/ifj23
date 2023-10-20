#include "test_utils.h"

void code_insert(char* test_code ) {
    FILE* test_file = fopen(TEST_FILE, "w");
    if (!test_file) {
        EPRINTF("Error opening testing input file\n");
    }

    fputs(test_code, test_file);
    fclose(test_file);
}

// line_or_column == "line" || "column"
void check_position( char *line_or_column, int correct_position[] ) {
    FILE* test_file = fopen(TEST_FILE, "r");
    if (!test_file) {
        EPRINTF("Error opening testing input file\n");
    }

    // Inicializace lexeru
    Stream in = stream_from_file(test_file, TEST_FILE);
    Lexer lexer = lex_new(in);

    Token token = lex_next(&lexer);
    bool test_flag = true;

    // Prováděj, dokud nebude vstupní .swift soubor prázdný
    for (int i = 0; token != T_ERR && token != EOF; i++) {
        // Pokud chci zpátky dostat číslo řádku
        if (strcmp(line_or_column, "line") == 0) {
            if (lexer.token_start.line == correct_position[i]) {
                token = lex_next(&lexer);
                continue;
            }
            // Pokud jsou nějaké nesrovnalosti
            else {
                test_flag = false;
                printf("\033[0;31m");
                printf("Expected line: %d\n"
                       "Acquired line: %zu\n\n", 
                       correct_position[i], lexer.token_start.line);
                printf("\033[0m");
            }
        }
        // Pokud chci zpátky dostat číslo sloupce
        else if (strcmp(line_or_column, "column") == 0) {
            if (lexer.token_start.column == correct_position[i]) {
                token = lex_next(&lexer);
                continue;
            }
            // Pokud jsou nějaké nesrovnalosti
            else {
                test_flag = false;
                printf("\033[0;31m");
                printf("Expected column: %d\n"
                       "Acquired column: %zu\n\n", 
                       correct_position[i], lexer.token_start.column);
                printf("\033[0m");
            }
        }
        else {
            test_flag = false;
            printf("\033[0;31mPlease enter whether to compare "
                   "line or column, test skipped\033[0m\n");
            break;
        }
        token = lex_next(&lexer);
    }

    lex_free(&lexer);
    fclose(test_file);

    test_eval(test_flag);
}

void check ( char *correct_tokens[] ) {
    FILE* test_file = fopen(TEST_FILE, "r");
        if (!test_file) {
            EPRINTF("Error opening testing input file\n");
    }

    Stream in = stream_from_file(test_file, TEST_FILE);
    Lexer lexer = lex_new(in);

    Token token = lex_next(&lexer);
    bool test_flag = true;

    // Prováděj, dokud nebude vstupní .swift soubor prázdný
    for (int i = 0; token != T_ERR && token != EOF; i++) {
        if (token == correct_tokens[i][0] || 
            strcmp(lexer.buffer.str, correct_tokens[i]) == 0) {
                token = lex_next(&lexer);
                continue;
        }
        // Pokud jsou nějaké nesrovnalosti
        else {
            test_flag = false;
            printf("\033[0;31m");
            // Pokud je to tisknutelný znak, vytiskne se
            if (isprint(token)) {
                printf("Expected token: %c\n"
                       "Acquired token: %c\n\n", 
                       correct_tokens[i][0], token);
            }
            // Pokud je to netisknutelný znak, vytiskne se string
            else {
                printf("Expected token: %s\n"
                       "Acquired token: %s\n\n", 
                       correct_tokens[i], lexer.buffer.str);
            }
            printf("\033[0m");
        }
        token = lex_next(&lexer);
    }
    lex_free(&lexer);
    fclose(test_file);

    test_eval(test_flag);
}

void run_test( char *correct_tokens[], char* test_code ) {
    code_insert(test_code);
    check(correct_tokens);
}

void test_eval( bool test_flag ) {
    if (test_flag) {
        printf("\033[0;32mTEST OK");
    }
    else {
        printf("\033[0;31mTEST FAILED");
    }
    printf("\033[0m");
}