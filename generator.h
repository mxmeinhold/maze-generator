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
    unsigned long rows;
    unsigned long cols;
    struct cell*** maze;
    tree_t neigh_tree;
};

struct cell {
    unsigned long row;
    unsigned long col;
    short num_wall;
    struct linked_list walls;
    short num_path;
    struct linked_list paths;
    char visited;
};

#define NUM_NEIGH 4

// allocates and fills a maze pointer
struct maze* gen_maze(unsigned long rows, unsigned long cols, void (*relocate)(struct cell*));

// deconstructs and frees the given maze pointer
void clean_maze(struct maze* input);

#endif
