#ifndef CODEGEN_H_INCLUDED
#define CODEGEN_H_INCLUDED

#include "inner_code.h"

#include <stdio.h>

bool cg_generate(Symtable *sym, InnerCode *code, FILE *out);

#endif // CODEGEN_H_INCLUDED
