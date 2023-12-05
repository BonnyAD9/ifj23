#include <stdio.h>
#include <ctype.h>

#include "utils.h"
#include "lexer.h"
#include "stream.h"
#include "symtable.h"
#include "vec.h"
#include "ast.h"
#include "parser.h"
#include "printer.h"
#include "debug_tools.h"
#include "inner_code.h"
#include "codegen.h"

int main(void) {
    FILE* file = fopen(DEBUG_FILE, "r");
    if (!file)
        EPRINTF("Error opening input file\n");

    Stream in = stream_from_file(file, DEBUG_FILE);

    // Init mock lexer, let him read input and output parsed tokens
    Lexer lexer = lex_new(in);
    Symtable table = sym_new();
    Parser parser = parser_new(&lexer, &table);
    AstBlock *block = parser_parse(&parser);
    parser_free(&parser);
    fclose(file);
    lex_free(&lexer);
    if (!block) {
        ast_free_block(&block);
        sym_free(&table);
        return get_first_err_code();
    }

    InnerCode ic;
    if (!ic_inner_code(&table, block, &ic)) {
        EPRINTF("Error generating inner code\n");
        ast_free_block(&block);
        sym_free(&table);
        return get_first_err_code();
    }

    FILE *res = fopen("res/res.ifjcode", "w");
    if (!cg_generate(&table, &ic, res)) {
        EPRINTF("Error generating target code\n");
        ast_free_block(&block);
        ic_free_code(&ic);
        sym_free(&table);
        return ERR_OTHER;
    }

    ast_free_block(&block);
    ic_free_code(&ic);
    sym_free(&table);

    return get_first_err_code();
}

bool sym_generate_builtins(Symtable *sym) {
    if (!sym_scope_add(sym)) {
        return OTHER_ERR_FALSE;
    }

    sym_declare(sym, STR("readString"), true);
    sym_func_new(DT_STRING_NIL, VEC_NEW(FuncParam));

    sym_declare(sym, STR("readInt"), true);
    sym_func_new(DT_INT_NIL, VEC_NEW(FuncParam));

    sym_declare(sym, STR("readDouble"), true);
    sym_func_new(DT_DOUBLE_NIL, VEC_NEW(FuncParam));

    sym_declare(sym, STR("write"), true);
    sym_func_new(DT_VOID, VEC_NEW(FuncParam));

    sym_declare(sym, STR("Int2Double"), true);
    sym_scope_add(sym);
    SymItem *term = sym_declare(sym, STR("term"), false);
    sym_var_new(DT_INT, false);
    sym_scope_pop(sym);
    sym_func_new(DT_DOUBLE, span_to_vec(SPAN_ARR(((FuncParam[]) {
        { .ident = term },
    }))));

    sym_declare(sym, STR("Double2Int"), true);
    sym_scope_add(sym);
    term = sym_declare(sym, STR("term"), false);
    sym_var_new(DT_DOUBLE, false);
    sym_scope_pop(sym);
    sym_func_new(DT_INT, span_to_vec(SPAN_ARR(((FuncParam[]) {
        { .ident = term },
    }))));

    sym_declare(sym, STR("length"), true);
    sym_scope_add(sym);
    SymItem *s = sym_declare(sym, STR("s"), false);
    sym_var_new(DT_STRING, false);
    sym_scope_pop(sym);
    sym_func_new(DT_INT, span_to_vec(SPAN_ARR(((FuncParam[]) {
        { .ident = s },
    }))));

    sym_declare(sym, STR("substring"), true);
    sym_scope_add(sym);
    s = sym_declare(sym, STR("s"), false);
    sym_var_new(DT_STRING, false);
    SymItem *i = sym_declare(sym, STR("i"), false);
    sym_var_new(DT_INT, false);
    SymItem *j = sym_declare(sym, STR("j"), false);
    sym_var_new(DT_INT, false);
    sym_scope_pop(sym);
    sym_func_new(DT_STRING_NIL, span_to_vec(SPAN_ARR(((FuncParam[]) {
        { .label = str_clone(STR("of")), .ident = s },
        { .label = str_clone(STR("startingAt")), .ident = i },
        { .label = str_clone(STR("endingBefore")), .ident = j },
    }))));

    sym_declare(sym, STR("ord"), true);
    sym_scope_add(sym);
    SymItem *c = sym_declare(sym, STR("c"), false);
    sym_var_new(DT_STRING, false);
    sym_scope_pop(sym);
    sym_func_new(DT_INT, span_to_vec(SPAN_ARR(((FuncParam[]) {
        { .ident = c },
    }))));

    sym_declare(sym, STR("chr"), true);
    sym_scope_add(sym);
    i = sym_declare(sym, STR("i"), false);
    sym_var_new(DT_INT, false);
    sym_scope_pop(sym);
    sym_func_new(DT_STRING, span_to_vec(SPAN_ARR(((FuncParam[]) {
        { .ident = i },
    }))));

    return true;
}
