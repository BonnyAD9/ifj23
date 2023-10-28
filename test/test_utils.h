#ifndef LEXER_UTILS_TEST
#define LEXER_UTILS_TEST

// Custom header files
#include "../src/lexer.h" // Working with the lexer
#include "../src/utils.h" // EPRINTF

// External header files
#include <stdio.h>
#include <stdbool.h> // bool
#include <ctype.h> // isprint()
#include <string.h> // strcmp()


#define TEST_FILE "test/test_file.swift"

// Colors
#define C_ESC "\x1b["
#define C_RESET C_ESC "0m"
#define C_GREEN(str) C_ESC "32m" str C_RESET
#define C_YELLOW(str) C_ESC "33m" str C_RESET
#define C_RED(str) C_ESC "31m" str C_RESET

typedef enum {
    LINE,
    COLUMN,
} LineOrCol;

typedef enum {
    NUMBER,
    STRING,
    ENUM_VALUE,
} EnumType;

typedef struct {
    EnumType enum_type;
    Token cur;
    char* str;
    double d_num;
    int i_num;
} TokenData;

/**
 * Updates the specific input file on which the lexer will perform
 * tests, and for each test loads new test code.
 * @param test_code Input code for testing
 */
int code_insert( char *test_code );

/**
 * Performs and prints the results of the position test to stdout
 * @param line_or_column What to print to the output and compare
 * @param correct_position Array of correct positions
 */
int check_position( LineOrCol line_or_column, int correct_position[] );

/**
 * Performs and prints the results of the token test to stdout
 * @param correct_tokens Array of TokenData structure values - expected values
 */
int check( TokenData correct_tokens[] );

/**
 * Helper function that combines void code_insert() and void check().
 * @param correct_tokens Array of TokenData structure values - expected values
 * @param test_code Input code for testing
 */
void run_test( TokenData correct_tokens[], char *test_code );

/**
 * Helper function that indicates how the test performed
 * @param test_flag Did it pass or fail?
 */
void test_eval( bool test_flag );

/**
 * Helper function that prints the difference between expected
 * and obtained tokens to stdout and returns false.
 * @param enum_type Token type where the error occurred
 * @param lexer 
 * @param correct_tokens Expected tokens 
 */
bool incorrect_token_print( EnumType enum_type, Lexer lexer, TokenData correct_tokens );

#endif 