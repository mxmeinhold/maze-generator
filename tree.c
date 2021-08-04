#include "tree.h"
#include <stdlib.h> // malloc(), free(), NULL

/* Metadata for a tree */
struct tree {
    struct tree_node* root; // the root node
    compare_func_t compare; // user defined compare function used to determine
                            // whether to place something in the left or the
                            // right child, or to detect duplicates
};

/* A tree node */
struct tree_node {
    void* data;              // the node data
    struct tree_node* left;  // the left child (null if empty)
    struct tree_node* right; // the right child (null if empty)
};

struct tree* new_tree(compare_func_t compare) {
    struct tree* tree = malloc(sizeof(struct tree_node));
    tree->root = NULL;
    tree->compare = compare;
    return tree;
}

/*
static struct tree_node* add(struct tree_node* node, void* data, compare_func_t compare) {
    if  (node == NULL) {
        struct tree_node* new_node = malloc(sizeof(struct tree_node));
        new_node->data = data;
        new_node->left = NULL;
        new_node->right = NULL;
        return new_node;
    }

    // Skip duplicates
    if (compare(node->data, data) > 0) {
        node->left = add(node->left, data, compare);
    } else if (compare(node->data, data) < 0) {
        node->right = add(node->right, data, compare);
    }

    return node;
}

void tree_add(struct tree* tree, void* data) {
    tree->root = add(tree->root, data, tree->compare);
}
*/

// Iterative version
void tree_add(struct tree* tree, void* data) {
    struct tree_node* current = tree->root;
    struct tree_node** last = &tree->root;
    while (current != NULL) {
        if (tree->compare(current->data, data) > 0) {
            last = &current->left;
            current = current->left;
        } else if (tree->compare(current->data, data) < 0) {
            last = &current->right;
            current = current->right;
        } else {
            // Skip duplicates
            return;
        }
    }

    struct tree_node* new_node = malloc(sizeof(struct tree_node));
    new_node->data = data;
    new_node->left = NULL;
    new_node->right = NULL;
    *last = new_node;
}

/*
static int contains(struct tree_node* node, void* data, compare_func_t compare) {
    if  (node == NULL) {
        return 0;
    }

    if (compare(node->data, data) > 0) {
        return contains(node->left, data, compare);
    } else if (compare(node->data, data) == 0) {
        return 1;
    } else {
        return contains(node->right, data, compare);
    }
}

int tree_contains(struct tree* tree, void* data) {
    return contains(tree->root, data, tree->compare);
}
*/

// Iterative version
int tree_contains(struct tree* tree, void* data) {
    struct tree_node* current = tree->root;
    while (current != NULL) {
        if (tree->compare(current->data, data) == 0) {
            return 1;
        } else if (tree->compare(current->data, data) > 0) {
           current = current->left;
        } else {
           current = current->right;
        }
    }

    return 0;
}

static void deallocate(struct tree_node* node) {
    if (node == NULL) {
        return;
    }

    deallocate(node->left);
    deallocate(node->right);
    free(node);
}

void tree_deallocate(struct tree* tree) {
    deallocate(tree->root);
    free(tree);
}

intmax_t basic_compare(const void* self, const void* other) {
    return ((intmax_t) self) - ((intmax_t) other);
}

/*
void* pop(struct tree_node* node) {
    if (node->left != NULL) {
        void* data = pop(node->left);
        if (data == NULL) {
            free(node->left);
            node->left = NULL;
            return pop(node);
        }
        return data;
    }
    if (node->right != NULL) {
        void* data = pop(node->right);
        if (data == NULL) {
            free(node->right);
            node->right = NULL;
            return pop(node);
        }
        return data;
    }

    void* data = node->data;
    node->data = NULL;
    return data;
}

void* tree_pop(tree_t tree) {
    void* data = pop(tree->root);
    if (data == NULL) {
        free(tree->root);
        tree->root = NULL;
    }
    return data;
}
*/

// Iterative version
void* tree_pop(tree_t tree) {
    struct tree_node* current = tree->root;
    struct tree_node** last = &tree->root;
    while (current != NULL) {
        if (current->left != NULL) {
            last = &current->left;
            current = current->left;
        } else if (current->right != NULL) {
            last = &current->right;
            current = current->right;
        } else {
            void* data = current->data;
            free(current);
            *last = NULL;
            return data;
        }
    }

    return NULL;
}
