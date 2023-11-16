#ifndef SYMTABLE_H_INCLUDED
#define SYMTABLE_H_INCLUDED

#include "str.h"
#include "stream.h"
#include "lexer.h"
#include "string.h"
#include "vec.h"

typedef enum {
    // Function
    SYM_FUNC,
    // Variable
    SYM_VAR,
    // Not defined
    SYM_NONE
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

typedef struct SymItem SymItem;

typedef struct {
    SymItem *ident;
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
struct SymItem {
    String name;
    Type type;
    union {
        FuncData func;
        VarData var;
    };
    unsigned int layer;
    // Marks position in file
    FilePos file_pos;
};

typedef struct node {
    const char *key;
    SymItem data;
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
void tree_insert(Tree *tree, const char *key, SymItem data);

/// Removes tree node by given key
void tree_remove(Tree *tree, const char *key);

/// Returns pointer to data of found node, if not found NULL
SymItem *tree_find(Tree *tree, const char *key);

/// Frees the tree - owner's responsibility!
void tree_free(Tree *tree);

void tree_visualise(Tree *tree);


/// Creates new symtable
Symtable sym_new();

/// Frees given symtable
void sym_free(Symtable *symtable);

/// Adds scope to the symtable and scope stack
void sym_scope_add(Symtable *symtable);

/// Pops scope from symtable scope stack
void sym_scope_pop(Symtable *symtable);

/// Creates new symtable item, without specifying its type
SymItem *sym_item_new(Symtable *symtable, String name, FilePos pos);

/// Sets ident to variable with given data
void sym_item_var(SymItem *ident, VarData var);

/// Sets ident to function with given data
void sym_item_func(SymItem *ident, FuncData fun);

/// Adds variable to the active scope (first in stack)
SymItem *sym_var_add(
    Symtable *symtable,
    String name,
    bool mutable,
    FilePos pos
);

/// Sets variable type
void sym_var_set_type(SymItem *var, DataType type, bool nullable);

/// Adds function to the current scope (first in stack)
SymItem *sym_func_add(Symtable *symtable, String name, FilePos pos);

/// Sets return type of given function
void sym_func_set_ret(SymItem *func, ReturnType ret);

/// Sets parameters of given function
void sym_func_set_params(SymItem *func, Vec params);

/// Gets return type of function with given name
ReturnType sym_func_get_ret(SymItem *data, String name);

/// Gets params of function with given name
Vec *sym_func_get_params(SymItem *data, String name);

/// Creates new function parameter
FuncParam sym_func_param_new(SymItem *ident, String label);

/// Creates new variable data
VarData sym_var_new(DataType type, bool nullable, bool mutable);

/// Creates new function data
FuncData sym_func_new(ReturnType ret, Vec params);

#endif // SYMTABLE_H_INCLUDED
