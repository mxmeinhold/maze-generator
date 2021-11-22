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
    unsigned long row;
    unsigned long col;
    unsigned long* coords;
    short num_wall;
    struct linked_list walls;
    short num_path;
    struct linked_list paths;
    char visited;
};

#define NUM_NEIGH 4

/**
 * Allocate and generate a three dimensional maze using 6-connected neighbors
 *
 * Any 2 cells who's coords differ by 1 and only 1 in 1 and only 1 dimension
 * are neighbors
 *
 * Args:
 * * rows: The number of rows in the maze
 * * cols: The number of columns in the maze
 * * depth: The number of cells in the third dimension
 * * limit: The limit on the number of iterations while generating the path
 *
 * Return: An allocated maze pointer. Deallocate using `clean_maze()`
 */
struct maze* gen_maze_3d_6(unsigned long rows, unsigned long cols, unsigned long depth, unsigned long limit);

/**
 * Allocate and generate a two dimensional maze using 4-connected neighbors
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
struct maze* gen_maze_4(unsigned long rows, unsigned long cols, void (*relocate)(struct cell*), unsigned long limit);

/**
 * Build a maze from a given starting node.
 *
 * This modifies the nodes in place, solving for a solution and marking walls
 * as paths via depth first search.
 *
 * This function has no dependencies on the number of neighbors a node has, so
 * mazes of arbitrary connectedness or size should be generatable.
 */
void gen_maze(struct cell* node, unsigned long limit);

// deconstructs and frees the given maze pointer
void clean_maze(struct maze* input);

#endif
