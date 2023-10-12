#ifndef SYMTABLE_H_INCLUDED
#define SYMTABLE_H_INCLUDED

#include "str.h"
#include "stream.h"
#include "lexer.h"
#include "string.h"

typedef enum {
    // Function
    FUNC,
    // Variable
    VAR
} Type;

typedef enum {
    LOCAL,
    GLOBAL
} Scope;

typedef enum {
    INT,
    DOUBLE,
    STRING
} DataType;

// Values saved in each tree node
typedef struct {
    // We don't expect name of var/func will change during runtime
    const char *name;
    Type type;
    // Marks position in file
    FilePos file_place;
    Scope scope;
    // Represents layer (aka depth) of where declared
    unsigned int layer;
    DataType data_type;
    // Function params string
    String func_params;
} NodeData;

typedef struct node {
    const char *key;
    NodeData data;
    struct node *left_node;
    struct node *right_node;
    // Height factor for maintaining tree balanced
    int height;
} TreeNode;

typedef struct {
    TreeNode *root_node;
} Tree;

/// Creates new tree
Tree tree_new();

/// Adds node into tree
void tree_insert(Tree *tree, const char *key, NodeData data);

/// Removes tree node by given key
void tree_remove(Tree *tree, const char *key);

/// Returns pointer to data of found node, if not found NULL
NodeData *tree_find(Tree *tree, const char *key);

/// Frees the tree - owner's responsibility!
void tree_free(Tree *tree);

void tree_visualise(Tree *tree);

#endif // SYMTABLE_H_INCLUDED
