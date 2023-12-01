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

int main(void) {
    FILE* file = fopen(DEBUG_FILE, "r");
    if (!file)
        EPRINTF("Error opening input file\n");

    Stream in = stream_from_file(file, DEBUG_FILE);

    // Init mock lexer, let him read input and output parsed tokens
    Lexer lexer = lex_new(in);
    Symtable table = sym_new();
    Parser parser = parser_new(&lexer, &table);
    AstBlock *res = parser_parse(&parser);
    parser_free(&parser);
    fclose(file);
    lex_free(&lexer);
    if (res) {
        print_ast_block(res, 1);
    }
    ast_free_block(&res);
    lex_free(&lexer);
    sym_free(&table);
}
