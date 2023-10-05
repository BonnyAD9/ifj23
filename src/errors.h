#ifndef ERRORS_H_INCLUDED
#define ERRORS_H_INCLDUED

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

#endif // ERRORS_H_INCLDUED
