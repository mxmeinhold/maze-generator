#ifndef MAZE_GEN_TREE_H
#define MAZE_GEN_TREE_H

#include <stdint.h> // intmax_t

/**
 * A compare function.
 * Positive return value selects the left child of self.
 * Negative return value selects the right child of self.
 * A 0 return inidcates that self and other are the same.
 */
typedef intmax_t(*compare_func_t)(const void* self, const void* other);

typedef struct tree* tree_t;

/**
 * Instantiate a new tree
 * compare is a compare function used to determine where to add new nodes to the tree.
 */
tree_t new_tree(compare_func_t compare);

/**
 * Adds a new node to the tree.
 * Note: this will deduplicate nodes that the compare function indicates are the same.
 */
void tree_add(tree_t tree, void* data);

/**
 * Test if `data` is contained in the tree.
 * Return 1 if it is, 0 otherwise.
 */
int tree_contains(tree_t tree, void* data);

/**
 * Free all the tree resources
 * Note that this won't mangle or free the data that had been in the tree.
 */
void tree_deallocate(tree_t tree);

/**
 * A basic compare function that treats self and other as integers.
 * Nothing fancy, but it serves as a fair default.
 */
intmax_t basic_compare(const void* self, const void* other);

/**
 * Pop data out of the tree.
 * Traversal order isn't guaranteed to be anything in particular, but this
 * works if you intend to use the tree as a set.
 */
void* tree_pop(tree_t tree);

#endif
