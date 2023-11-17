#ifndef SEMANTICS_H_INCLUDED
#define SEMANTICS_H_INCLUDED

#include <stdbool.h> // bool

#include "ast.h"   // AstStmt
#include "enums.h" // DataType

bool check_statement(AstStmt *stmt);

typedef struct {
    bool in_func;
    bool in_main;
    bool in_while;
    bool in_if;
    bool return_used;
    DataType func_type;
} Context;

#endif // SEMANTICS_H_INCLUDED
