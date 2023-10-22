#ifndef LEXER_TEST
#define LEXER_TEST

#include "test_utils.h"

#define TEST_FILE "test/test_file.swift"
#define LEX_TEST_COUNT ((long int) (sizeof(lex_tests) / sizeof(*lex_tests)))

/**
 * Vypíše do výstupního souboru nadpis k dannému testu
 * @param NAME Název testu
 * @param DESCRIPTION Popis testu
 */
#define TEST(NAME, DESCRIPTION)                     \
void NAME(void) {                                   \
    printf("[%s] - %s\n", #NAME, DESCRIPTION);      \

/**
 * Ukončí test
 */
#define ENDTEST                                     \
    printf("\n");                                   \
}

#endif