#ifndef SYMTABLE_H_INCLUDED
#define SYMTABLE_H_INCLUDED

#include "str.h"
#include "stream.h"
#include "lexer.h"
#include "string.h"
#include "vec.h"
#include "enums.h"
#include "errors.h"
#include "utils.h"
#include "debug_tools.h"

typedef enum {
    // Function
    SYM_FUNC,
    // Variable
    SYM_VAR,
    // Not defined
    SYM_NONE
} Type;

typedef enum {
    LOCAL,
    GLOBAL
} Scope;

typedef struct SymItem SymItem;

typedef struct {
    SymItem *ident;
    String label;
} FuncParam;

typedef struct {
    DataType return_type;
    // type: FuncParam
    Vec params;
} FuncData;

typedef struct {
    DataType data_type;
    DataType original_data_type; // Will be used just for if() statements with "let id" as condition, dont use anyhow else!
    unsigned int counter;        // Will be used just for if() statements with "let id" as condition, dont use anyhow else!
    bool mutable;
    bool initialized;
} VarData;

// Values saved in each tree node
struct SymItem {
    String name;
    String uname;
    Type type;
    bool global;
    union {
        FuncData func;
        VarData var;
    };
    bool declared;
    // Marks position in file
    FilePos file_pos;
};

typedef struct node {
    String key;
    // TODO: change to SymItem *
    // SymItem vector
    Vec data;
    struct node *left_node;
    struct node *right_node;
    // Height factor for maintaining tree balanced
    int height;
} TreeNode;

typedef struct {
    TreeNode *root_node;
    int id;
} Tree;

typedef struct {
    Vec scopes;
    Vec scope_stack;
} Symtable;

/// Creates new tree
Tree tree_new(int id);

/// Adds node into tree
bool tree_insert(Tree *tree, const String key, Vec data);

/// Removes tree node by given key
bool tree_remove(Tree *tree, const String key);

/// Returns pointer to data of found node, if not found NULL
Vec *tree_find(Tree *tree, const String key);

/// Frees the tree - owner's responsibility!
void tree_free(Tree **tree);

void tree_visualise(Tree *tree);

//===========================================================================//
//                                 Symtable                                  //
//===========================================================================//

/// Creates new symtable
Symtable sym_new();

/// Frees given symtable
void sym_free(Symtable *symtable);

/// Adds scope to the symtable and scope stack
bool sym_scope_add(Symtable *symtable);

/// Pops scope from symtable scope stack
void sym_scope_pop(Symtable *symtable);

/// Finds item in symtable, create new if not existing
SymItem *sym_find(Symtable *symtable, String name);

/// Declares new item
SymItem *sym_declare(Symtable *symtable, String name, bool is_function);

/// Sets ident to variable with given data
void sym_item_var(SymItem *ident, VarData var);

/// Sets ident to function with given data
void sym_item_func(SymItem *ident, FuncData fun);

/// Gets return type of function with given name
DataType sym_func_get_ret(SymItem *data, String name);

/// Gets params of function with given name
Vec *sym_func_get_params(SymItem *data, String name);

/// Creates new function parameter
FuncParam sym_func_param_new(SymItem *ident, String label);

/// Creates new variable data
VarData sym_var_new(DataType type, bool mutable);

/// Creates new function data
FuncData sym_func_new(DataType ret, Vec params);

/// Frees func param
void sym_free_func_param(FuncParam *par);

// Generates unique temporary variable with the given type
SymItem *sym_tmp_var(Symtable *symtable, DataType type);

// Generates temporary function (same as `sym_tmp_var` but doesn't require
// type, and sets the type to function)
SymItem *sym_label(Symtable *symtable);

#endif // SYMTABLE_H_INCLUDED
