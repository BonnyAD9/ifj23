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
}


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
