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
static TreeNode *create_node(const char *key, NodeData data);
/// Process right rotation with given tree node
static TreeNode *right_rotate(TreeNode *y);
/// Process left rotation with given tree node
static TreeNode *left_rotate(TreeNode *y);
/// Updates node's height and returns its balance factor
static int _update_height_ret_balance(TreeNode *node);
/// Recursively delete tree
static void _tree_free(TreeNode *node);
/// Recursively search for node with given key
static NodeData *_tree_find(TreeNode *node, const char *key);
/// Inserts new node into current tree
static TreeNode *_tree_insert(TreeNode *node, const char *key, NodeData data);
/// Deletes node in current tree
static TreeNode *_tree_remove(TreeNode *node, const char *key);
/// Prints tree structure and node values to stdout
static void _visualise_tree(TreeNode *node);

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
    str_free(&node->data.name);
    if (node->data.type == FUNC)
        vec_free(&node->data.func.params);
    free(node);
    node = NULL;
}

void tree_free(Tree *tree) {
    _tree_free(tree->root_node);
}

static NodeData *_tree_find(TreeNode *node, const char *key) {
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

NodeData *tree_find(Tree *tree, const char *key) {
    return _tree_find(tree->root_node, key);
}

static TreeNode *create_node(const char *key, NodeData data) {
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

static TreeNode *_tree_insert(TreeNode *node, const char *key, NodeData data) {
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

void tree_insert(Tree *tree, const char *key, NodeData data) {
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



Symtable symtable_new() {
    return (Symtable) {
        .scopes = VEC_NEW(Tree),
        .scope_stack = VEC_NEW(Tree)
    };
}

void symtable_free(Symtable *symtable) {
    VEC_FOR_EACH(&symtable->scopes, Tree*, scope) {
        tree_free(*scope.v);
        free(*scope.v);
        scope.v = NULL;
    }
    vec_free(&symtable->scopes);
    vec_free(&symtable->scope_stack);
}

void symtable_scope_add(Symtable *symtable) {
    Tree *new = malloc(sizeof(Tree));
    new->root_node = NULL;

    VEC_PUSH(&symtable->scopes, Tree*, new);
    VEC_PUSH(&symtable->scope_stack, Tree*, new);
}

void symtable_scope_pop(Symtable *symtable) {
    VEC_POP(&symtable->scope_stack, Tree*);
}

NodeData *symtable_var_add(
    Symtable *symtable,
    String name,
    bool mutable,
    FilePos pos
) {
    Tree *scope = VEC_LAST(&symtable->scope_stack, Tree*);

    NodeData data = {
        .name = name,
        .type = VAR,
        .var = {
            .data_type = NONE,
            .nullable = false,
            .mutable = mutable,
        },
        .file_place = pos
    };
    tree_insert(scope, name.str, data);
    NodeData *var = tree_find(scope, name.str);
    return var;
}

void symtable_var_set_type(NodeData *var, DataType type, bool nullable) {
    if (var->type != VAR)
        return;

    var->var.data_type = type;
    var->var.nullable = nullable;
}

NodeData *symtable_func_add(Symtable *symtable, String name, FilePos pos) {
    Tree *scope = VEC_LAST(&symtable->scope_stack, Tree*);

    NodeData data = {
        .name = name,
        .type = FUNC,
        .func = {
            .return_type = RET_VOID
        },
        .file_place = pos
    };
    tree_insert(scope, name.str, data);
    NodeData *func = tree_find(scope, name.str);
    return func;
}

void symtable_func_set_return(NodeData *func, ReturnType ret) {
    if (func->type != FUNC)
        return;

    func->func.return_type = ret;
}

void symtable_func_set_params(NodeData *func, Vec params) {
    if (func->type != FUNC)
        return;

    func->func.params = params;
}

ReturnType symtable_func_get_return(NodeData *data, String name) {
    if (data->type != FUNC)
        return RET_VOID;

    return data->func.return_type;
}

Vec *symtable_func_get_params(NodeData *data, String name) {
    if (data->type != FUNC)
        return NULL;

    return &data->func.params;
}
