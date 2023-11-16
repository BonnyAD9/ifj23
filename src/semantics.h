#ifndef SEMANTICS_H_INCLUDED
#define SEMANTICS_H_INCLUDED

#include "stdbool.h"
#include "string.h"
#include "ast.h"
#include "errors.h"
#include "vec.h"
#include "utils.h"

bool check_statement(AstStmt *stmt);

typedef struct {
    bool in_func;
    bool in_main;
    bool in_while;
    bool in_if;
    bool return_used;
    AstFuncType func_type;
} Context;

#endif // SEMANTICS_H_INCLUDED