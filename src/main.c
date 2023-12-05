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
