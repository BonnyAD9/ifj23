#include <stdio.h>
#include <ctype.h>

#define DEBUG_FILE "test/testInput.swift"

#include "utils.h"
#include "lexer.h"
#include "stream.h"
#include "symtable.h"

void print_node_data(Tree *tree, NodeData **data, const char *key) {
    *data = tree_find(tree, key);
    if (*data)
        // Also print data stored in found node
        fprintf(stdout, "Node %s found, data.layer=%d\n", key, (*data)->layer);
    else
        fprintf(stdout, "Node %s NOT found\n", key);
}

int main(void) {
    FILE* file = fopen(DEBUG_FILE, "r");
    if (!file)
        EPRINTF("Error opening input file\n");

    Stream in = stream_from_file(file, DEBUG_FILE);

    // Init mock lexer, let him read input and output parsed tokens
    Lexer lexer = lex_new(in);
    Token token = lex_next(&lexer);

    while (token != T_ERR && token != EOF) {
        if (isprint(token)) {
            printf(
                DEBUG_FILE ":%zu:%zu: Token %c|%d [%s]\n",
                lexer.token_start.line,
                lexer.token_start.column,
                token,
                token,
                lexer.buffer.str
            );
        }
        else {
            printf(
                DEBUG_FILE ":%zu:%zu: Token |%d [%s]\n",
                lexer.token_start.line,
                lexer.token_start.column,
                token,
                lexer.buffer.str
            );
        }
        token = lex_next(&lexer);
    }
    lex_free(&lexer);
    printf("----------------------------------------\n");

    // Symtable - tree tests
    Tree tree = tree_new();
    //////////////// Insertion ////////////////
    tree_insert(&tree, "A", (NodeData){.layer = 1});
    tree_insert(&tree, "B", (NodeData){.layer = 2});
    tree_insert(&tree, "C", (NodeData){.layer = 3});
    tree_insert(&tree, "D", (NodeData){.layer = 4});
    tree_insert(&tree, "E", (NodeData){.layer = 5});
    tree_insert(&tree, "F", (NodeData){.layer = 6});
    // Print tree content
    tree_visualise(&tree);
    fprintf(stdout, "\n");
    //////////////// Deletion ////////////////
    tree_remove(&tree, "C");
    tree_remove(&tree, "E");
    tree_remove(&tree, "@");
    // Print tree content
    tree_visualise(&tree);
    //////////////// Lookup ////////////////
    NodeData *data;
    print_node_data(&tree, &data, "F");
    // Try to modify node's data, layer=6 -> layer=0
    if (data)
        data->layer = 0;
    // Print again and expect data.layer value change
    print_node_data(&tree, &data, "F");
    // Modify node's layer by insertion
    tree_insert(&tree, "F", (NodeData){.layer = 1});
    print_node_data(&tree, &data, "F");
    // Try to find non-existing node
    print_node_data(&tree, &data, "@");

    tree_free(&tree);
    fclose(file);
}
