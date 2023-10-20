#ifndef LEXER_UTILS_TEST
#define LEXER_UTILS_TEST

// Vlastní hlavičkové soubory
#include "../src/lexer.h" // práce s lexerem
#include "../src/utils.h" // EPRINTF

// Externí hlavičkové soubory
#include <stdio.h>
#include <stdbool.h> // bool
#include <ctype.h> // isprint()
#include <string.h> // strcmp()


#define TEST_FILE "test/test_file.swift"

/**
 * Aktualizuje konkrétní vstupní soubor, na kterém bude lexer provádět
 * testy a při každém dalším testu nahraje nový testovaný kód
 * @param test_code Vstupní kód na testování
 */
void code_insert( char *test_code );

/**
 * Provádí a vypíše do stdout výsledky testu pozice
 * @param line_or_column True pokud chci tisknout LINE, false pokud COLUMN
 */
void check_position( char *line_or_column, int correct_position[] );

/**
 * Provádí a vypíše do stdout výsledky testu tokenů
 * @param correct_tokens Pole očekávaných stringů, které má lexer vrátit
 */
void check( char *correct_tokens[] );

/**
 * Pomocná funkce, která spojuje void code_insert() a void check()
 * @param correct_tokens Pole očekávaných stringů, které má lexer vrátit
 * @param test_code Vstupní kód na testování
 */
void run_test( char *correct_tokens[], char *test_code );

/**
 * Pomocná funkce, která řekne, jak test dopadl
 * @param test_flag Dopadl dobře nebo špatně?
 */
void test_eval( bool test_flag );

#endif 