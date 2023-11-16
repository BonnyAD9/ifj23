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
    bool declared;
    // Marks position in file
    FilePos file_pos;
};

typedef struct node {
    const char *key;
    // SymItem vector
    Vec data;
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
void tree_insert(Tree *tree, const char *key, Vec data);

/// Removes tree node by given key
void tree_remove(Tree *tree, const char *key);

/// Returns pointer to data of found node, if not found NULL
Vec *tree_find(Tree *tree, const char *key);

/// Frees the tree - owner's responsibility!
void tree_free(Tree *tree);

void tree_visualise(Tree *tree);

//===========================================================================//
//                                 Symtable                                  //
//===========================================================================//

/// Creates new symtable
Symtable sym_new();

/// Frees given symtable
void sym_free(Symtable *symtable);

/// Adds scope to the symtable and scope stack
void sym_scope_add(Symtable *symtable);

/// Pops scope from symtable scope stack
void sym_scope_pop(Symtable *symtable);

/// Finds item in symtable, create new if not existing
SymItem *sym_find(Symtable *symtable, String name);

/// Declares new item
SymItem *sym_declare(Symtable *symtable, String name);

/// Sets ident to variable with given data
void sym_item_var(SymItem *ident, VarData var);

/// Sets ident to function with given data
void sym_item_func(SymItem *ident, FuncData fun);

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
