#ifndef LEXER_TEST
#define LEXER_TEST

#include "test_utils.h"

#define TEST_FILE "test/test_file.swift"
/**
 * Vypíše do výstupního souboru nadpis k dannému testu
 * @param which_to_print Název testu
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