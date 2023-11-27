#include "symtable.h"
#include <stdio.h>
#include <stdlib.h>

/// Returns height of a node (number of edges in the longest path connecting the node to any leaf node)
static int node_height(TreeNode *node);
/// Returns balance of a node (balance = Height of Left Subtree - Height of Right Subtree)
static int node_balance(TreeNode *node);
/// Returns bigger of two passed-in values
static int max(int a_val, int b_val);
/// Create new tree node with given key and data
static TreeNode *create_node(const char *key, Vec data);
/// Process right rotation with given tree node
static TreeNode *right_rotate(TreeNode *y);
/// Process left rotation with given tree node
static TreeNode *left_rotate(TreeNode *y);
/// Updates node's height and returns its balance factor
static int _update_height_ret_balance(TreeNode *node);
/// Recursively delete tree
static void _tree_free(TreeNode *node);
/// Recursively search for node with given key
static Vec *_tree_find(TreeNode *node, const char *key);
/// Inserts new node into current tree
static TreeNode *_tree_insert(TreeNode *node, const char *key, Vec data);
/// Deletes node in current tree
static TreeNode *_tree_remove(TreeNode *node, const char *key);
/// Prints tree structure and node values to stdout
static void _visualise_tree(TreeNode *node);

/// Frees SymItem vector
void sym_data_free(Vec *data);

static int node_height(TreeNode *node) {
    return ((node) ? node->height : 0);
}

static int node_balance(TreeNode *node) {
    if (!node)
        return 0;
    return (node_height(node->left_node) - node_height(node->right_node));
}

static int max(int a_val, int b_val) {
    return ((a_val < b_val) ? b_val : a_val);
}

Tree tree_new() {
    return (Tree) {
        .root_node = NULL
    };
}

static void _tree_free(TreeNode *node) {
    if (!node)
        return;
    // Recursively free left and right subtrees
    _tree_free(node->left_node);
    _tree_free(node->right_node);

/*
    // Release allocated function params
    str_free(&node->data.func_params);
*/
    sym_data_free(&node->data);
    free(node);
    node = NULL;
}

void tree_free(Tree *tree) {
    _tree_free(tree->root_node);
}

static Vec *_tree_find(TreeNode *node, const char *key) {
    // Search failed
    if (!node)
        return NULL;
    int strcmp_val = strcmp(key, node->key);
    // Matching node found
    if (!strcmp_val)
        return &node->data;
    // Key value is > node's key (in ASCII) -> go to right subtree
    if (strcmp_val > 0)
        return _tree_find(node->right_node, key);
    // Key value is < node's key (in ASCII) -> go to left subtree
    return _tree_find(node->left_node, key);
}

Vec *tree_find(Tree *tree, const char *key) {
    return _tree_find(tree->root_node, key);
}

static TreeNode *create_node(const char *key, Vec data) {
    TreeNode *new_node = malloc(sizeof(TreeNode));
    if (!new_node)
        return NULL;

    // Copy given key to node key
    new_node->key = key;

    // Default value for leaf
    new_node->height = 1;
    new_node->data = data;
    // Initially no subtrees
    new_node->left_node = NULL;
    new_node->right_node = NULL;

    return new_node;
}

static TreeNode *right_rotate(TreeNode *y){
    TreeNode *x = y->left_node;
    y->left_node = x->right_node;
    x->right_node = y;

    // Height values update
    x->height = max(node_height(x->left_node), node_height(x->right_node)) + 1;
    y->height = max(node_height(y->left_node), node_height(y->right_node)) + 1;

    return x;
}

static TreeNode *left_rotate(TreeNode *y){
    TreeNode *x = y->right_node;
    y->right_node = x->left_node;
    x->left_node = y;

    // Height values update
    x->height = max(node_height(x->left_node), node_height(x->right_node)) + 1;
    y->height = max(node_height(y->left_node), node_height(y->right_node)) + 1;

    return x;
}

static int _update_height_ret_balance(TreeNode *node) {
    node->height = max(node_height(node->left_node), node_height(node->right_node)) + 1;
    return node_balance(node);
}

static TreeNode *_tree_insert(TreeNode *node, const char *key, Vec data) {
    if (!node)
        return create_node(key, data);

    int strcmp_val = strcmp(key, node->key);
    // Key value > node's key (in ASCII) -> go to right subtree
    if (strcmp_val > 0)
        node->right_node = _tree_insert(node->right_node, key, data);
    // Key value < node's key (in ASCII) -> go to left subtree
    else  if (strcmp_val < 0)
        node->left_node = _tree_insert(node->left_node, key, data);
    // Update node's value
    else
        node->data = data;

    int n_balance = _update_height_ret_balance(node);
    // Check if parent node became unbalanced
    // node_balance > 1 or < -1
    if (abs(n_balance) > 1) {
        if (n_balance > 1) {
            strcmp_val = strcmp(key, node->left_node->key);
            // Key value < node left's key
            if (strcmp_val < 0)
                return right_rotate(node);
            // Key value > node left's key
            if (strcmp_val > 0) {
                node->left_node = left_rotate(node->left_node);
                return right_rotate(node);
            }
        }
        if (n_balance < -1) {
            strcmp_val = strcmp(key, node->right_node->key);
            // Key value > node right's key
            if (strcmp_val > 0)
                return left_rotate(node);
            // Key value < node right's key
            if (strcmp_val < 0) {
                node->right_node = right_rotate(node->right_node);
                return left_rotate(node);
            }
        }
    }
    return node;
}

void tree_insert(Tree *tree, const char *key, Vec data) {
    tree->root_node = _tree_insert(tree->root_node, key, data);
    if (!tree->root_node)
        return;
}

static TreeNode *_tree_remove(TreeNode *node, const char *key) {
    if (!node)
        return node;

    int strcmp_val = strcmp(key, node->key);
    // Key value > node's key (in ASCII) -> go to right subtree
    if (strcmp_val > 0)
        node->right_node = _tree_remove(node->right_node, key);
    // Key value < node's key (in ASCII) -> go to left subtree
    else  if (strcmp_val < 0)
        node->left_node = _tree_remove(node->left_node, key);
    // We found node for deletion
    else {
        TreeNode *temp;
        // Node with only one child - bridge over to it's right child and free node
        if (!node->left_node) {
            temp = node;
            node = node->right_node;
            free(temp);
        }
        // Node with only one child - bridge over to it's left child and free node
        else if (!node->right_node) {
            temp = node;
            node = node->left_node;
            free(temp);
        }
        // Node with both children
        else {
            temp = node;
            // Find biggest node in left subtree
            temp = temp->left_node;
            while (temp->right_node->right_node)
                temp = temp->right_node;

            node->key = temp->right_node->key;
            // Delete subtree of found node
            node->left_node = _tree_remove(node->left_node, node->key);
        }
    }
    // Node might have change, check to avoid NULL pointer dereferention(s)
    if (!node)
        return node;

    int n_balance = _update_height_ret_balance(node);
    // Check if parent node became unbalanced
    // node_balance > 1 or < -1
    if (abs(n_balance) > 1) {
        if (n_balance > 1) {
            if (node_balance(node->left_node) >= 0)
                return right_rotate(node);

            if (node_balance(node->left_node) < 0) {
                node->left_node = left_rotate(node->left_node);
                return right_rotate(node);
            }
        }
        if (n_balance < -1) {
            if (node_balance(node->right_node) <= 0)
                return left_rotate(node);

            if (node_balance(node->right_node) > 0) {
                node->right_node = right_rotate(node->right_node);
                return left_rotate(node);
            }
        }
    }
    return node;
}

void tree_remove(Tree *tree, const char *key) {
    tree->root_node = _tree_remove(tree->root_node, key);
    if (!tree->root_node)
        return;
}

static void _visualise_tree(TreeNode *node) {
    if (node) {
        // Print node' key
        fprintf(stdout, "| %s |\n", node->key);
        // Recursively print rest of tree
        _visualise_tree(node->left_node);
        _visualise_tree(node->right_node);
    }
}

void tree_visualise(Tree *tree) {
    fprintf(stdout, "[root] - ");
    _visualise_tree(tree->root_node);
}

//===========================================================================//
//                                 Symtable                                  //
//===========================================================================//

Symtable sym_new() {
    return (Symtable) {
        .scopes = VEC_NEW(Tree),
        .scope_stack = VEC_NEW(Tree*)
    };
}

void sym_free(Symtable *symtable) {
    vec_free_with(&symtable->scopes, (FreeFun)tree_free);
    vec_free(&symtable->scopes);
    vec_free(&symtable->scope_stack);
}

void sym_data_free(Vec *data) {
    VEC_FOR_EACH(data, SymItem, item) {
        str_free(&item.v->name);
        if (item.v->type == SYM_FUNC) {
            vec_free_with(&item.v->func.params, (FreeFun)sym_free_func_param);
            vec_free(&item.v->func.params);
        }

    }
}

void sym_scope_add(Symtable *symtable) {
    Tree new = tree_new();

    VEC_PUSH(&symtable->scopes, Tree, new);
    VEC_PUSH(
        &symtable->scope_stack,
        Tree*,
        &VEC_LAST(&symtable->scopes, Tree)
    );
}

void sym_scope_pop(Symtable *symtable) {
    vec_pop(&symtable->scope_stack);
}

SymItem *sym_find(Symtable *symtable, String name) {
    Tree *scope = NULL;
    for (int i = symtable->scope_stack.len - 1; i >= 0; --i) {
        scope = VEC_AT(&symtable->scope_stack, Tree*, i);
        Vec *data = tree_find(scope, name.str);

        if (data)
            return &VEC_LAST(data, SymItem);
    }

    if (!scope)
        return NULL;

    Vec new = VEC_NEW(SymItem);
    name = str_clone(name);
    SymItem item = {
        .name = name,
        .type = SYM_NONE,
        .declared = false,
    };
    VEC_PUSH(&new, SymItem, item);

    tree_insert(scope, name.str, new);
    return &VEC_LAST(&new, SymItem);
}

SymItem *sym_declare(Symtable *symtable, String name, bool is_function) {
    Tree *scope = VEC_LAST(&symtable->scope_stack, Tree*);
    if (!scope)
        return NULL;

    Vec *data = tree_find(scope, name.str);
    Type new_type = is_function ? SYM_FUNC : SYM_VAR;

    if (!data) {
        Vec new = VEC_NEW(SymItem);
        name = str_clone(name);
        SymItem item = {
            .name = name,
            .type = new_type,
            .declared = true,
        };
        VEC_PUSH(&new, SymItem, item);

        tree_insert(scope, name.str, new);
        return &VEC_LAST(&new, SymItem);
    }

    SymItem *item = &VEC_LAST(data, SymItem);
    if (!item->declared) {
        item->type = new_type;
        item->declared = true;
        return item;
    }

    // Doesn't allow redeclaration in global scope
    if (symtable->scope_stack.len == 1)
        return NULL;

    name = str_clone(name);
    SymItem new_item = {
        .name = name,
        .type = new_type,
        .declared = true,
    };
    VEC_PUSH(data, SymItem, new_item);
    return &VEC_LAST(data, SymItem);
}

void sym_item_var(SymItem *ident, VarData var) {
    ident->type = SYM_VAR;
    ident->var = var;
}

void sym_item_func(SymItem *ident, FuncData fun) {
    ident->type = SYM_FUNC;
    ident->func = fun;
}

DataType sym_func_get_ret(SymItem *data, String name) {
    if (data->type != SYM_FUNC)
        return DT_NONE;

    return data->func.return_type;
}

Vec *sym_func_get_params(SymItem *data, String name) {
    if (data->type != SYM_FUNC)
        return NULL;

    return &data->func.params;
}

FuncParam sym_func_param_new(SymItem *ident, String label) {
    return (FuncParam) {
        .ident = ident,
        .label = label,
    };
}

VarData sym_var_new(DataType type, bool mutable) {
    return (VarData) {
        .data_type = type,
        .mutable = mutable
    };
}

FuncData sym_func_new(DataType ret, Vec params) {
    return (FuncData) {
        .return_type = ret,
        .params = params
    };
}

void sym_free_func_param(FuncParam *par) {
    str_free(&par->label);
}
