#ifndef MAZE_GEN_GENERATOR_H
#define MAZE_GEN_GENERATOR_H

#include "tree.h"

#define MAZE_WALL '*'
#define MAZE_SPACE '+'

struct maze {
    unsigned short rows;
    unsigned short cols;
    struct cell*** maze;
    tree_t neigh_tree;
};
struct cell {
    short row;
    short col;
    short num_neigh;
    struct neigh** neighs;
};

struct neigh {
    short wall;
    struct cell* left;
    struct cell* right;
    short row;
    short col;
};

#define NUM_NEIGH 4

// allocates and fills a maze pointer
struct maze* gen_maze(unsigned short rows, unsigned short cols, void (*relocate)(struct cell*));

// deconstructs and frees the given maze pointer
void clean_maze(struct maze* input);

#endif
