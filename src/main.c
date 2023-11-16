#include <stdio.h>
#include <ctype.h>

#define DEBUG_FILE "test/testInput.swift"

#include "utils.h"
#include "lexer.h"
#include "stream.h"
#include "symtable.h"
#include "vec.h"

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

    printf("----------------------------------------\n");
    // vector test
    Vec v = VEC_NEW(int);

    vec_push_span(&v, SPAN_ARR(((int []) { 5, 4, 3, 2, 1 })));

    VEC_PUSH(&v, int, 0);

    VEC_FOR_EACH(&v, int, item) {
        printf("%zu: %d\n", item.i, *item.v);
    }

    printf("pop %d, set [3]=10 and set last = 7\n", VEC_POP(&v, int));
    VEC_AT(&v, int, 3) = 10;
    VEC_LAST(&v, int) = 7;

    VEC_FOR_EACH(&v, int, item) {
        printf("%zu: %d\n", item.i, *item.v);
    }

    printf("push tail of vec to the vec\n");

    Span tail = vec_slice(&v, 1, v.len - 1);


    printf("tail of vec:\n");
    SPAN_FOR_EACH(tail, int, item) {
        printf("%zu: %d\n", item.i, *item.v);
    }

    Vec v2 = span_to_vec(tail);
    vec_push_span(&v, vec_as_span(&v2));
    vec_free(&v2);

    printf("Vec:\n");
    VEC_FOR_EACH(&v, int, item) {
        printf("%zu: %d\n", item.i, *item.v);
    }

    vec_free(&v);

    Symtable symtable = symtable_new();
    symtable_scope_add(&symtable);
    FilePos pos = {
        .column = 0,
        .line = 0
    };
    data = symtable_var_add(&symtable, str_clone(STR("x")), true, pos);
    symtable_var_set_type(data, INT, false);
    Tree *scope = VEC_LAST(&symtable.scope_stack, Tree*);

    // tree_visualise(&VEC_LAST(&symtable.scope_stack, Tree));
    symtable_free(&symtable);
}
