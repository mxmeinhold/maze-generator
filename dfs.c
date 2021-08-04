#include "generator.h"
#include "stack.h"
#include "tree.h"
#include <stdlib.h> // malloc(), free(), rand()

struct neigh* make_neigh(struct cell* left, struct cell* right) {
    struct neigh* neigh = malloc(sizeof(struct neigh));
    neigh->wall = (short) MAZE_WALL;
    neigh->left = left;
    neigh->right = right;
    neigh->row = left->row + (right->row - left->row)/2;
    neigh->col = left->col + (right->col - left->col)/2;
    return neigh;
}

static void shuffle(struct neigh** array, size_t n) {
    for (size_t i = 0; i < n - 1; i++) {
        size_t j = i + (size_t)rand() / (RAND_MAX / (n - i) + 1);
        struct neigh* tmp = array[j];
        array[j] = array[i];
        array[i] = tmp;
    }
}

struct maze* gen_maze(unsigned short rows, unsigned short cols, void (*relocate)(struct cell*)) {
    // Init a grid
    struct maze* out = malloc(sizeof(struct maze));
    out->rows = rows;
    out->cols = cols;
    out->maze = calloc(sizeof(struct cell**), rows);
    for (short row = 0; row < out->rows; row++) {
        out->maze[row] = calloc(sizeof(struct cell*), cols);
    }

    // Fill it
    for (short r = 0; r < rows; r++) {
        for (short c = 0; c < cols; c++) {
            struct cell* new = malloc(sizeof(struct cell));
            out->maze[r][c] = new;
            new->row = r;
            new->col = c;
            relocate(new);// Ugh TODO
            new->num_neigh = 0;
            new->neighs = calloc(sizeof(struct neigh*), NUM_NEIGH);
        }
    }

    // Link neighbors (only to the right and down)
    // We'll have to do the right and bottom edges still
    for (short r = 0; r < rows - 1; r++) {
        for (short c = 0; c < cols - 1; c++) {
            struct neigh* down = make_neigh(out->maze[r][c], out->maze[r+1][c]);
            out->maze[r+1][c]->neighs[out->maze[r+1][c]->num_neigh++] = down;
            out->maze[r][c]->neighs[out->maze[r][c]->num_neigh++] = down;
            struct neigh* right = make_neigh(out->maze[r][c], out->maze[r][c+1]);
            out->maze[r][c+1]->neighs[out->maze[r][c+1]->num_neigh++] = right;
            out->maze[r][c]->neighs[out->maze[r][c]->num_neigh++] = right;
        }
    }

    // Right edge
    for (short r = 0; r < rows - 1; r++) {
        short c = (short)cols - 1;
        struct neigh* down = make_neigh(out->maze[r][c], out->maze[r+1][c]);
        out->maze[r+1][c]->neighs[out->maze[r+1][c]->num_neigh++] = down;
        out->maze[r][c]->neighs[out->maze[r][c]->num_neigh++] = down;
    }

    // Bottom edge
    for (short c = 0; c < cols - 1; c++) {
        short r = (short)rows - 1;
        struct neigh* right = make_neigh(out->maze[r][c], out->maze[r][c+1]);
        out->maze[r][c+1]->neighs[out->maze[r][c+1]->num_neigh++] = right;
        out->maze[r][c]->neighs[out->maze[r][c]->num_neigh++] = right;
    }

    int counts[] = {0,0,0,0,0,0,0,0,0};
    for (short r = 0; r < rows; r++) {
        for (short c = 0; c < cols; c++) {
            struct cell* n = out->maze[r][c];
            counts[n->num_neigh] += 1;
            // Randomize neighs
            shuffle(n->neighs, (size_t)n->num_neigh);
        }
    }

    // Create a maze
    stack_t stack = new_stack();
    tree_t visited_tree = new_tree(&basic_compare);

    // pick a random cell
    struct cell* node = out->maze[rand() % out->rows][rand() % out->cols];
    // mark visited
    tree_add(visited_tree, (void*)node);
    // push to stack
    stack_push(stack, node);
    while (stack_peek(stack)) {
        // pop
        node = (struct cell*) stack_pop(stack);
        // pick an unvisited neighbor
        for (short n = 0; n < node->num_neigh; n++) {
            struct cell* next;
            if (node->neighs[n]->left != node) {
                next = node->neighs[n]->left;
            } else {
                next = node->neighs[n]->right;
            }
            if (!tree_contains(visited_tree, (void*)next)) {
                stack_push(stack, node);
                // remove the wall
                node->neighs[n]->wall = MAZE_SPACE;
                // mark as visited and push to stack
                tree_add(visited_tree, (void*)next);
                stack_push(stack, next);
                break;
            }
        }
    }

    tree_deallocate(visited_tree);
    stack_deallocate(stack);
    return out;
}

void clean_maze(struct maze* input) {
    tree_t tree = new_tree(&basic_compare);
    for (short r = 0; r < input->rows; r++) {
        for (short c = 0; c < input->cols; c++) {
            struct cell* node = input->maze[r][c];
            for (int n = 0; n < node->num_neigh; n++) {
                tree_add(tree, node->neighs[n]);
            }
            free(node->neighs);
            free(node);
        }
        free(input->maze[r]);
    }
    free(input->maze);
    free(input);

    void* data;
    while ((data = tree_pop(tree)) != NULL) free(data);
    tree_deallocate(tree);
}
