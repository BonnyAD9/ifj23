#include "test_utils.h"

int code_insert(char* test_code ) {
    FILE* test_file = fopen(TEST_FILE, "w");
    if (!test_file) {
        EPRINTF("Error opening testing input file\n");
        exit(EXIT_FAILURE);
    }

    fputs(test_code, test_file);
    fclose(test_file);
    return EXIT_SUCCESS;
}

// line_or_column == "line" || "column"
int check_position( LineOrCol line_or_column, int correct_position[] ) {
    FILE* test_file = fopen(TEST_FILE, "r");
    if (!test_file) {
        EPRINTF("Error opening testing input file\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the lexer
    Stream in = stream_from_file(test_file, TEST_FILE);
    Lexer lexer = lex_new(in);

    Token token = lex_next(&lexer);
    bool test_flag = true;

    // Perform until the input .swift file is empty
    for (int i = 0; token != T_ERR && token != EOF; i++) {
        if (line_or_column == LINE && lexer.token_start.line == correct_position[i]) {
            token = lex_next(&lexer);
            continue;
        }
        if (line_or_column == COLUMN && lexer.token_start.column == correct_position[i]) {
            token = lex_next(&lexer);
            continue;
        }
        // If there is an error
        test_flag = false;
        printf(C_RED(
                    "Expected %s: %d\n"
                    "Acquired %s: %zu\n\n"),
                    (line_or_column == LINE) ? "line" : "column",
                    correct_position[i],
                    (line_or_column == LINE) ? "line" : "column",
                    (line_or_column == LINE) ? lexer.token_start.line : lexer.token_start.column
                    );

        token = lex_next(&lexer);
    }

    lex_free(&lexer);
    fclose(test_file);

    test_eval(test_flag);
    return EXIT_SUCCESS;
}

int check( TokenData correct_tokens[] ) {
    FILE* test_file = fopen(TEST_FILE, "r");
        if (!test_file) {
            EPRINTF("Error opening testing input file\n");
            exit(EXIT_FAILURE);
    }

    // Initialize the lexer
    Stream in = stream_from_file(test_file, TEST_FILE);
    Lexer lexer = lex_new(in);

    Token token = lex_next(&lexer);
    bool test_flag = true;

    // Perform until the input .swift file is empty
    for (int i = 0; token != T_ERR && token != EOF; i++) {

        switch(correct_tokens[i].enum_type) {
        // If it is an ENUM_TYPE:
        case ENUM_VALUE:
            if (correct_tokens[i].cur != lexer.cur) {
                test_flag = incorrect_token_print(ENUM_VALUE, lexer, correct_tokens[i]);
            }
            break;

        // If it is a STRING
        case STRING:
            if (correct_tokens[i].cur != lexer.cur ||
                strcmp(correct_tokens[i].str, lexer.buffer.str) != 0) {

                test_flag = incorrect_token_print(STRING, lexer, correct_tokens[i]);
            }
            break;

        // If it is a NUMBER
        case NUMBER:
           // INTEGER
            if (correct_tokens[i].cur == T_ILIT) {
                if (correct_tokens[i].cur != lexer.cur ||
                    correct_tokens[i].i_num != lexer.i_num) {

                    test_flag = incorrect_token_print(NUMBER, lexer, correct_tokens[i]);
                }
            }
            // If it is a DOUBLE
            else if (correct_tokens[i].cur == T_DLIT) {
                if (correct_tokens[i].cur != lexer.cur ||
                    correct_tokens[i].d_num != lexer.d_num) {

                    test_flag = incorrect_token_print(NUMBER, lexer, correct_tokens[i]);
                }
            }
            break;

        default:
        test_flag = incorrect_token_print(correct_tokens[i].enum_type,
                                         lexer, correct_tokens[i]);
        }

        token = lex_next(&lexer);
    }

    lex_free(&lexer);
    fclose(test_file);

    test_eval(test_flag);
    return EXIT_SUCCESS;
}

void run_test( TokenData correct_tokens[], char* test_code ) {
    code_insert(test_code);
    check(correct_tokens);
}

void test_eval( bool test_flag ) {
    if (test_flag) {
        printf(C_GREEN("TEST OK"));
    }
    else {
        printf(C_RED("TEST FAILED"));
    }
    printf("\033[0m");
}

bool incorrect_token_print( EnumType enum_type, Lexer lexer, TokenData correct_tokens ) {
    if (enum_type == ENUM_VALUE) {
        printf(C_RED(
            "Expected token: .cur[%c]\n"
            "Aquired token:  .cur[%c]\n\n"),
            correct_tokens.cur, lexer.cur
        );
    }

    else if (enum_type == STRING) {
        printf(C_RED(
            "Expected token: .cur[%d], .str[%s]\n"
            "Aquired token:  .cur[%d], .str[%s]\n\n"),
            correct_tokens.cur, correct_tokens.str, lexer.cur, lexer.buffer.str
        );
    }

    else if (enum_type == NUMBER) {
        // INT
        if (correct_tokens.cur == T_ILIT && lexer.cur == T_ILIT) {
            printf(C_RED(
                "Expected token: .cur[%d], .i_num[%d]\n"
                "Aquired token:  .cur[%d], .i_num[%d]\n\n"),
                correct_tokens.cur, correct_tokens.i_num, lexer.cur, lexer.i_num
            );
        }
        // DOUBLE
        else if (correct_tokens.cur == T_DLIT && lexer.cur == T_DLIT) {
            printf(C_RED(
                "Expected token: .cur[%d], .d_num[%f]\n"
                "Aquired token:  .cur[%d], .d_num[%f]\n\n"),
                correct_tokens.cur, correct_tokens.d_num, lexer.cur, lexer.d_num
            );
        }
        // ERROR
        else {
            printf(C_RED(
                "Expected token: .cur[%d]\n"
                "Aquired token:  .cur[%d]\n"
                "Cannot compare different .cur\n\n"),
                correct_tokens.cur, lexer.cur
            );
        }
    }
    return false;
}