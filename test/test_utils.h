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

// Barvy
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
 * Aktualizuje konkrétní vstupní soubor, na kterém bude lexer provádět
 * testy a při každém dalším testu nahraje nový testovaný kód
 * @param test_code Vstupní kód na testování
 */
int code_insert( char *test_code );

/**
 * Provádí a vypíše do stdout výsledky testu pozice
 * @param line_or_column Co chceme vytisknout na výstup a porovnat 
 * @param correct_position Vstupní pole správných pozic
 */
int check_position( LineOrCol line_or_column, int correct_position[] );

/**
 * Provádí a vypíše do stdout výsledky testu tokenů
 * @param correct_tokens Pole hodnot struktury TokenData - očekávané hodnoty 
 */
int check( TokenData correct_tokens[] );

/**
 * Pomocná funkce, která spojuje void code_insert() a void check()
 * @param correct_tokens Pole hodnot struktury TokenData - očekávané hodnoty 
 * @param test_code Vstupní kód na testování
 */
void run_test( TokenData correct_tokens[], char *test_code );

/**
 * Pomocná funkce, která řekne, jak test dopadl
 * @param test_flag Dopadl dobře nebo špatně?
 */
void test_eval( bool test_flag );

/**
 * Pomocná funkce, která na výstup vytiskne oslišnost 
 * očekávaného tokenu od získaného a vrátí false 
 * @param enum_type Typ tokenu, ve kterém se vyskytla chyba
 * @param lexer 
 * @param correct_tokens Očekávané tokeny 
 */
bool incorect_token_print( EnumType enum_type, Lexer lexer, TokenData correct_tokens );

#endif 