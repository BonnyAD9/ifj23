// Vlastní hlavičkové soubory
#include "../src/utils.h"
#include "../src/lexer.h"
#include "../src/stream.h"

// Externí hlavičkové soubory
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#define TEST_FILE "test/test_file.swift"

/**
 * Aktualizuje konkrétní vstupní soubor, na kterém bude lexer provádět testy a
 * při každém dalším testu nahraje nový testovaný kód (pokaždé se bude v TEST volat)
 * @param test_code Vstupní kód na testování
 */
void code_insert(char* test_code );

/**
 * Vypíše do výstupního souboru umístění tokenu
 * @param which_to_print True pokud chci tisknout LINE, false pokud COLUMN
 */
void print_location( bool which_to_print );

/**
 * Vypíše do výstupního souboru zpracovávaný token
 */
void print_token(  );
