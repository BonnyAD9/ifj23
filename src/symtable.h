#ifndef SYMTABLE_H_INCLUDED
#define SYMTABLE_H_INCLUDED

#include "str.h"
#include "stream.h"
#include "lexer.h"
#include "string.h"
#include "vec.h"

typedef enum {
    // Function
    FUNC,
    // Variable
    VAR
} Type;

typedef enum {
    RET_INT,
    RET_DOUBLE,
    RET_STRING,
    RET_VOID
} ReturnType;

typedef enum {
    LOCAL,
    GLOBAL
} Scope;

typedef enum {
    INT,
    DOUBLE,
    STRING,
    NONE
} DataType;

typedef struct {
    String name;
    String label;
} FuncParam;

typedef struct {
    ReturnType return_type;
    Vec params;
} FuncData;

typedef struct {
    DataType data_type;
    bool nullable;
    bool mutable;
} VarData;

// Values saved in each tree node
typedef struct {
    String name;
    Type type;
    union {
        FuncData func;
        VarData var;
    };
    unsigned int layer;
    // Marks position in file
    FilePos file_place;
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

typedef struct {
    Vec scopes;
    Vec scope_stack;
} Symtable;

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


/// Creates new symtable
Symtable symtable_new();

/// Frees given symtable
void symtable_free(Symtable *symtable);

/// Adds scope to the symtable and scope stack
void symtable_scope_add(Symtable *symtable);

/// Pops scope from symtable scope stack
void symtable_scope_pop(Symtable *symtable);

/// Adds variable to the active scope (first in stack)
NodeData *symtable_var_add(
    Symtable *symtable,
    String name,
    bool mutable,
    FilePos pos
);

/// Sets variable type
void symtable_var_set_type(NodeData *var, DataType type, bool nullable);

/// Adds function to the current scope (first in stack)
NodeData *symtable_func_add(Symtable *symtable, String name, FilePos pos);

/// Sets return type of given function
void symtable_func_set_return(NodeData *func, ReturnType ret);

/// Sets parameters of given function
void symtable_func_set_params(NodeData *func, Vec params);

/// Gets return type of function with given name
ReturnType symtable_func_get_return(NodeData *data, String name);

/// Gets params of function with given name
Vec *symtable_func_get_params(NodeData *data, String name);

#endif // SYMTABLE_H_INCLUDED
