#include "generator.h"
#include "stack.h"
#include "tree.h"
#include <stdlib.h> // malloc(), free(), rand()

static void shuffle(struct linked_list* list) {
    // Make an array to make shuffling easier
    size_t size = 0;
    for (struct list_node* node = list->start; node != NULL; node = node->next) size++;

    if (size > 0) {
        struct list_node** array = calloc(sizeof(struct list_node*), size);
        size_t i = 0;
        for (struct list_node* node = list->start; node != NULL; node = node->next) {
            array[i++] = node;
        }

        // Shuffle
        for (i = 0; i < size - 1; i++) {
            size_t j = i + (size_t)rand() / (RAND_MAX / (size - i) + 1);
            struct list_node* tmp = array[j];
            array[j] = array[i];
            array[i] = tmp;
        }

        // Rebuild the linked list
        list->start = array[0];
        array[0]->prev = NULL;
        array[size-1]->next = NULL;
        if (size > 1) {
            array[0]->next = array[1];
            array[size-1]->prev = array[size-2];
        }
        for (i = 1; i < size-1; i++) {
            array[i]->next = array[i+1];
            array[i]->prev = array[i-1];
        }

        free(array);
    }
}

struct maze* gen_maze(unsigned long rows, unsigned long cols, void (*relocate)(struct cell*)) {
    // Init a grid
    struct maze* out = malloc(sizeof(struct maze));
    out->rows = rows;
    out->cols = cols;
    out->maze = calloc(sizeof(struct cell**), rows);
    for (unsigned long row = 0; row < out->rows; row++) {
        out->maze[row] = calloc(sizeof(struct cell*), cols);
    }

    // Fill it
    for (unsigned long r = 0; r < rows; r++) {
        for (unsigned long c = 0; c < cols; c++) {
            struct cell* new = malloc(sizeof(struct cell));
            out->maze[r][c] = new;
            new->row = r;
            new->col = c;
            new->walls.end = NULL;
            new->walls.start = NULL;
            new->paths.end = NULL;
            new->paths.start = NULL;
            relocate(new);// Ugh TODO
        }
    }

    // Link neighbors (only to the right and down)
    // We'll have to do the right and bottom edges still
    for (unsigned long r = 0; r < rows - 1; r++) {
        for (unsigned long c = 0; c < cols - 1; c++) {
            struct cell* current = out->maze[r][c];
            struct cell* down = out->maze[r+1][c];
            list_push(&current->walls, down);
            list_push(&down->walls, current);

            struct cell* right = out->maze[r][c+1];
            list_push(&current->walls, right);
            list_push(&right->walls, current);
        }
    }

    // Right edge
    for (unsigned long r = 0; r < rows - 1; r++) {
        unsigned long c = cols - 1;
        struct cell* current = out->maze[r][c];
        struct cell* down = out->maze[r+1][c];
        list_push(&current->walls, down);
        list_push(&down->walls, current);
    }

    // Bottom edge
    for (unsigned long c = 0; c < cols - 1; c++) {
        unsigned long r = rows - 1;
        struct cell* current = out->maze[r][c];
        struct cell* right = out->maze[r][c+1];
        list_push(&current->walls, right);
        list_push(&right->walls, current);
    }

    for (unsigned long r = 0; r < rows; r++) {
        for (unsigned long c = 0; c < cols; c++) {
            struct cell* n = out->maze[r][c];
            // Randomize walls
            shuffle(&n->walls);
        }
    }

    // Create a maze
    stack_t stack = new_stack();
    tree_t visited_tree = new_tree(&basic_compare);

    // pick a random cell
    // TODO save and return this? something something the maze is a tree?
    struct cell* node = out->maze[(unsigned long)rand() % out->rows][(unsigned long)rand() % out->cols];
    // mark visited
    tree_add(visited_tree, (void*)node);
    // push to stack
    intmax_t stack_size = 0;
    stack_push(stack, node); stack_size++;
    while (stack_peek(stack)) {
        // pop
        node = (struct cell*) stack_pop(stack); stack_size--;
        // pick an unvisited neighbor
        struct list_node* wall = node->walls.start;
        for (; wall != NULL;) {
            struct list_node* next = wall->next;
            if (!tree_contains(visited_tree, (void*)wall->cell)) {
                stack_push(stack, node); stack_size++;
                // remove the wall
                list_remove(&node->walls, wall);
                list_remove_data(&wall->cell->walls, node); // This will will leave us with a tree of paths
                wall->next = node->paths.start;
                node->paths.start = wall;
                if (node->paths.end == NULL) node->paths.end = wall;
                // mark as visited and push to stack
                tree_add(visited_tree, (void*)wall->cell);
                stack_push(stack, wall->cell); stack_size++;
                break;
            }
            wall = next;
        }
    }

    tree_deallocate(visited_tree);
    stack_deallocate(stack);
    return out;
}

void clean_maze(struct maze* input) {
    for (unsigned long r = 0; r < input->rows; r++) {
        for (unsigned long c = 0; c < input->cols; c++) {
            struct cell* node = input->maze[r][c];
            list_deallocate(&node->walls);
            list_deallocate(&node->paths);
            free(node);
        }
        free(input->maze[r]);
    }
    free(input->maze);
    free(input);
}

