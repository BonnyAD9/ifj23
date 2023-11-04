#include <stdio.h>
#include <ctype.h>

#define DEBUG_FILE "test/testInput.swift"

#include "utils.h"
#include "lexer.h"
#include "stream.h"
#include "symtable.h"
#include "vec.h"
#include "ast.h"

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
    fclose(file);
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

    printf("----------------------------------------\n");
    // AST Basic Tests

    // pom : String = 5
    AbstractSyntaxTree* var_tree = make_variable_Expr(STR("pom"),
                                                      STRING_VALUE,
                                                      make_valueExpr(NUL_STR,
                                                                     INT_VALUE,
                                                                     &((Lexer) {.i_num = 5})
                                                                    )
                                                     );
    preorder_traversal(var_tree);
    free_ast(var_tree);

    // pom : String = funcA(ahoj, svete);
    Vec arg_vec = VEC_NEW(AbstractSyntaxTree*);
    VEC_PUSH(&arg_vec, AbstractSyntaxTree*, make_valueExpr(STR("ahoj"),
                                                           STRING_VALUE,
                                                           &((Lexer) {.str = STR("ahoj")}) // Real value will be found in symtable
                                                          ));
    VEC_PUSH(&arg_vec, AbstractSyntaxTree*, make_valueExpr(STR("svete"),
                                                           STRING_VALUE,
                                                           &((Lexer) {.str = STR("svete")})
                                                          ));

    var_tree = make_variable_Expr(STR("pom"),
                                  STRING_VALUE,
                                  make_function_callExpr(STR("funcA"), &arg_vec));
    preorder_traversal(var_tree);
    free_ast(var_tree);

    /*
        if (a < b) {
            a = 8;
            b = 10 - a;
        }
        else {
            return funcB(jeden);
        }
    */
    Vec if_body = VEC_NEW(AbstractSyntaxTree*);
    VEC_PUSH(&if_body, AbstractSyntaxTree*, make_variable_Expr(STR("a"),
                                                               INT_VALUE,
                                                               make_valueExpr(NUL_STR,
                                                                              INT_VALUE,
                                                                              &((Lexer) {.i_num = 8})
                                                                             )
                                                              ));
    VEC_PUSH(&if_body, AbstractSyntaxTree*, make_variable_Expr(STR("b"),
                                                               INT_VALUE,
                                                               make_binaryExpr(STR("-"),
                                                                               make_valueExpr(NUL_STR, INT_VALUE, &((Lexer) {.i_num = 10})),
                                                                               make_valueExpr(NUL_STR, STRING_VALUE, &((Lexer) {.str = STR("a")})))
                                                              ));

    Vec func_arg = VEC_NEW(AbstractSyntaxTree*);
    VEC_PUSH(&func_arg, AbstractSyntaxTree*, make_valueExpr(STR("jeden"),
                                                            STRING_VALUE,
                                                            &((Lexer) {.str = STR("jeden")})
                                                           ));
    Vec else_body = VEC_NEW(AbstractSyntaxTree*);
    VEC_PUSH(&else_body, AbstractSyntaxTree*, make_returnExpr(make_function_callExpr(STR("funcB"), &func_arg)));

    AbstractSyntaxTree* cond = make_conditionExpr(make_binaryExpr(STR("<"),
                                                                  make_variable_Expr(STR("a"), STRING_VALUE, NULL),
                                                                  make_variable_Expr(STR("b"), STRING_VALUE, NULL)),
                                                  &if_body,
                                                  &else_body);
    preorder_traversal(cond);
    free_ast(cond);

    /*
        while (a == b) {
            print("Dobry den");
        }
    */
    func_arg = VEC_NEW(AbstractSyntaxTree*);
    VEC_PUSH(&func_arg, AbstractSyntaxTree*, make_valueExpr(STR("Dobry den"),
                                                            STRING_VALUE,
                                                            &((Lexer) {.str = STR("Dobry den")})
                                                           ));
    Vec while_body = VEC_NEW(AbstractSyntaxTree*);
    VEC_PUSH(&while_body, AbstractSyntaxTree*, make_function_callExpr(STR("print"), &func_arg));

    AbstractSyntaxTree* while_cyc = make_whileExpr(make_binaryExpr(STR("=="),
                                                                   make_variable_Expr(STR("a"), STRING_VALUE, NULL),
                                                                   make_variable_Expr(STR("b"), STRING_VALUE, NULL)),
                                                   &while_body);
    preorder_traversal(while_cyc);
    free_ast(while_cyc);

    /*
        func func_def (name name_param : String, age age_param : Int) -> Int {
            let a = "jenom jeden radek";
        }
    */
    Vec params_vec = VEC_NEW(AbstractSyntaxTree*);
    VEC_PUSH(&params_vec, AbstractSyntaxTree*, make_parameterExpr(STR("name"),
                                                                  STR("name_param"),
                                                                  STRING_VALUE));
    VEC_PUSH(&params_vec, AbstractSyntaxTree*, make_parameterExpr(STR("age"),
                                                                  STR("age_param"),
                                                                  INT_VALUE));

    Vec body_vec = VEC_NEW(AbstractSyntaxTree*);
    VEC_PUSH(&body_vec, AbstractSyntaxTree*, make_variable_Expr(STR("a"),
                                                                STRING_VALUE,
                                                                make_valueExpr(STR("a"),
                                                                               STRING_VALUE,
                                                                               &((Lexer) {.str = STR("jenom jeden radek")}))
                                                               ));

    AbstractSyntaxTree* func = make_function_defExpr(STR("func_def"),
                                                     &params_vec,
                                                     &body_vec,
                                                     INT_VALUE);
    preorder_traversal(func);
    free_ast(func);

}