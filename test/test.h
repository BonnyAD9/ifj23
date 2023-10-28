#ifndef LEXER_TEST
#define LEXER_TEST

#include "test_utils.h"

#define TEST_FILE "test/test_file.swift"
#define LEX_TEST_COUNT ((long int) (sizeof(lex_tests) / sizeof(*lex_tests)))

/**
 * Prints the heading for a specific test to the output file
 * @param NAME Test name
 * @param DESCRIPTION Test description
 */
#define TEST(NAME, DESCRIPTION)                     \
void NAME(void) {                                   \
    printf("[%s] - %s\n", #NAME, DESCRIPTION);      \

/**
 * Ends the test
 */
#define ENDTEST                                     \
    printf("\n");                                   \
}

#endif