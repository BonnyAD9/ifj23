/**
 * IFJ23
 *
 * xdanie14 Tomáš Daniel
 * xsleza23 Martin Slezák
 * xstigl00 Jakub Antonín Štigler
 */

#ifndef ERRORS_H_INCLUDED
#define ERRORS_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

/// no error
#define SUCCESS

/// Error during lexical analisis
#define ERR_LEX 1

/// Error during syntactic analisis
#define ERR_SYNTAX 2

/// Semantic error - undefined function
#define ERR_UNDEF_FUNCTION 3

/// Semantic error - invalid number / type of parameter or return value
#define ERR_INVALID_FUN 4

/// Semantic error - undefined variable
#define ERR_UNDEF_VAR 5

/// Semantic error - function returns incorrectly
#define ERR_INVALID_RETURN 6

/// Semantic error - incompatible types in aritmetic operation
#define ERR_INCOM_TYPE 7

/// Semantic error - cannot infer type of variable
#define ERR_UNKNOWN_TYPE 8

/// Other semantic error
#define ERR_SEMANTIC 9

/// Other error
#define ERR_OTHER 99

#define OTHER_ERR_FALSE (other_err(__FILE__, __LINE__), false)
#define OTHER_ERR_NULL (other_err(__FILE__, __LINE__), NULL)

// returns false if the expression is false
#define CHECK(...) if (!(__VA_ARGS__)) return false
// declares variable and returns false if it is not true
#define CHECKD(type, name, ...) \
    type name = (__VA_ARGS__); \
    if (!name) return false

// returns NULL if the expression is false
#define CHECKN(...) if (!(__VA_ARGS__)) return NULL
// declares variable and returns null if it is not true
#define CHECKND(type, name, ...) \
    type name = (__VA_ARGS__); \
    if (!name) return NULL

void set_err_code(int err_code);

int get_first_err_code();

int get_last_err_code();

void other_err(char *file, int line);

#endif // ERRORS_H_INCLDUED
