#include <stdio.h>
#include <ctype.h>

#define DEBUG_FILE "test/testInput.swift"

#include "utils.h"
#include "lexer.h"
#include "stream.h"
#include "symtable.h"

int main(void) {
    FILE* file = fopen(DEBUG_FILE, "r");
    if (!file)
        EPRINTF("Error opening input file\n");

    Stream in = stream_from_file(file, DEBUG_FILE);

    // Init mock lexer, let him read input and output parsed tokens
    Lexer lexer = lex_new(in);
    Token token = lex_next(&lexer);

    printf(DEBUG_FILE ":\n");
    while (token != T_ERR && token != EOF) {
        if (isprint(token)) {
            printf(
                "   %zu:%zu: Token %c|%d [%s]\n",
                lexer.token_start.line,
                lexer.token_start.column,
                token,
                token,
                lexer.str.str
            );
        }
        else {
            printf(
                "   %zu:%zu: Token |%d [%s]\n",
                lexer.token_start.line,
                lexer.token_start.column,
                token,
                lexer.str.str
            );
        }
        token = lex_next(&lexer);
    }
    lex_free(&lexer);
    printf("----------------------------------------\n");

    // Symtable - tree tests
    Tree tree = tree_new();
    NodeData data;
    //////////////// Insertion ////////////////
    tree_insert(&tree, "A", data);
    tree_insert(&tree, "B", data);
    tree_insert(&tree, "C", data);
    tree_insert(&tree, "D", data);
    tree_insert(&tree, "E", data);
    tree_insert(&tree, "F", data);
    // Print tree content
    tree_visualise(&tree);
    fprintf(stdout, "\n");
    //////////////// Deletion ////////////////
    tree_remove(&tree, "C");
    tree_remove(&tree, "E");
    tree_remove(&tree, "@");
    // Print tree content
    tree_visualise(&tree);
    bool found = tree_find(&tree, "F", &data);
    fprintf(stdout, "Node 'F' %s\n", ((found) ? "found" : "NOT found"));
    found = tree_find(&tree, "#", &data);
    fprintf(stdout, "Node '#' %s", ((found) ? "found" : "NOT found"));

    tree_free(&tree);
}
