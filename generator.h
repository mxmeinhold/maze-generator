#ifndef MAZE_GEN_GENERATOR_H
#define MAZE_GEN_GENERATOR_H

#include "tree.h"
#include <stdlib.h> // size_t

#define MAZE_WALL '*'
#define MAZE_SPACE '+'

// TODO these should be in a separate header
struct list_node {
    struct list_node* next;
    struct list_node* prev;
    struct cell* cell;
};

struct linked_list {
    struct list_node* start;
    struct list_node* end;
    size_t length;
};

void list_push(struct linked_list* list, struct cell* cell);
void list_remove(struct linked_list* list, struct list_node* node);
void list_remove_data(struct linked_list* list, struct cell* cell);
void list_deallocate(struct linked_list* list);

struct maze {
    unsigned dims;
    long unsigned* dims_array;
    struct cell*** maze;
};

struct cell {
    struct linked_list walls;
    struct linked_list paths;
    unsigned long* coords;
    unsigned long row;
    unsigned long col;
    short num_wall;
    short num_path;
    char visited;
};

#define NUM_NEIGH 4

/**
 * Allocate a maze
 *
 * This is a helper function for allocating a maze's datastructures. Useful if
 * you're looking to do some form of custom generation, otherwise see the
 * `gen_maze_*` functions below.
 *
 * Args:
 * - dims: length of `dims_array`
 * - dims_array: array of sizes of maze dimensions
 *
 * Return: The allocated maze. Deallocate using `clean_maze()`
 */
struct maze* alloc_maze(unsigned dims, unsigned long* dims_array);

/**
 * Get a cell from the maze
 *
 * _THIS FUNCTION DOES NO BOUNDS CHECKING_
 * You must validate the coordinates you pass this function yourself.
 *
 * Args:
 * - maze: the maze to look in
 * - coords: the coordinates of the cell you want, in the order of `maze->dim_array`
 *
 * Return: Pointer to the requested cell
 *
 */
struct cell* get_cell(struct maze* maze, unsigned long* coords);

/**
 * Link each cell in the maze to its neighbors, populating the `walls` list of
 * each cell
 *
 * Any two cells who's coordinates differ by exactly 1 in 1 and only 1
 * dimension are neighbors.
 */
void link_neighs(struct maze* maze);

/**
 * Allocate and generate a three dimensional maze using 6-connected neighbors
 *
 * Uses the definition of neighbors from `link_neighs()`
 *
 * Args:
 * * rows: The number of rows in the maze
 * * cols: The number of columns in the maze
 * * depth: The number of cells in the third dimension
 * * limit: The limit on the number of iterations while generating the path
 *
 * Return: An allocated maze pointer. Deallocate using `clean_maze()`
 */
struct maze* gen_maze_3d_6(unsigned long rows, unsigned long cols, unsigned long depth, unsigned long limit, void (*write_step)(const struct maze*, const struct cell*, unsigned int));

/**
 * Allocate and generate a two dimensional maze using 4-connected neighbors
 *
 * Uses the definition of neighbors from `link_neighs()`
 *
 * Args:
 * * rows: The number of rows in the maze
 * * cols: The number of columns in the maze
 * * relocate: A function to remap a cell's row and column. This is mostly for
 *             convenience for rendering purposes
 * * limit: The limit on the number of iterations while generating the path
 *
 * Return: An allocated maze pointer. Deallocate using `clean_maze()`
 */
struct maze* gen_maze_4(unsigned long rows, unsigned long cols, void (*relocate)(struct cell*), unsigned long limit, void (*write_step)(const struct maze*, const struct cell*, unsigned int));

/**
 * Build a maze from a given starting node.
 *
 * This modifies the nodes in place, solving for a solution and marking walls
 * as paths via depth first search.
 *
 * This function has no dependencies on the number of neighbors a node has, so
 * mazes of arbitrary connectedness or size should be generatable.
 */
void gen_maze(struct cell* node, unsigned long limit, struct maze* maze, void (*write_step)(const struct maze*, const struct cell*, unsigned int));

// deconstructs and frees the given maze pointer
void clean_maze(struct maze* input);

#endif
