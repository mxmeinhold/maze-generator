#include "generator.h"
#include "stack.h"
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

// Recursively allocate a maze, with one recursion layer per dimension
struct cell** alloc_dim(unsigned dims, unsigned long* dims_array) {
    if (dims == 0) {
        // Init a node
        struct cell* new = calloc(sizeof(struct cell), 1);
        return (struct cell**) new;
    } else {
        struct cell** this = calloc(sizeof(struct cell**), dims_array[0]);
        for (unsigned long i = 0; i < dims_array[0]; i++) {
            this[i] = (struct cell*)alloc_dim(dims - 1, dims_array + 1);
        }
        return this;
    }
}

// Alocate a maze
struct maze* alloc_maze(unsigned dims, unsigned long* dims_array) {
    struct maze* out = malloc(sizeof(struct maze));
    out->dims = dims;
    out->dims_array = calloc(sizeof(unsigned long), dims);
    for (unsigned d = 0; d < dims; d++) out->dims_array[d] = dims_array[d];
    out->maze = (struct cell***)alloc_dim(dims, dims_array);
    return out;
}

// Get a cell that is located at `coords` from `maze`
// Coords are used in the order of `maze->dims_array`
struct cell* get_cell(struct maze* maze, unsigned long* coords) {
    struct cell** current = (struct cell**) maze->maze;
    for (unsigned d = 0; d < maze->dims; d++) {
        current = (struct cell**)current[coords[d]];
    }
    return (struct cell*) current;
}

// Link each cell in the maze to its neighbors
//
// Neighbors are defined by:
// Any two cells who's coordinates differ by exactly 1 in exactly 1 dimension are neighbors.
void link_neighs(struct maze* maze) {
    unsigned long* coords = calloc(sizeof(unsigned long), maze->dims);
    struct cell* cell;
    struct cell* other;
    while (1) {
        cell = get_cell(maze, coords);

        // Set coords
        cell->coords = calloc(sizeof(unsigned long), maze->dims);
        for (unsigned c = 0; c < maze->dims; c++) cell->coords[c] = coords[c];

        // Link neighbors
        for (unsigned d = 0; d < maze->dims; d++) {
            if (coords[d] + 1 < maze->dims_array[d]) {
                // plus 1
                coords[d]++;
                other = get_cell(maze, coords);
                list_push(&cell->walls, other);
                coords[d]--;
            }
            if (coords[d] - 1 < maze->dims_array[d]) {
                // minus 1 (unsigned, so we're checking for underflow)
                coords[d]--;
                other = get_cell(maze, coords);
                list_push(&cell->walls, other);
                coords[d]++;
            }
        }

        shuffle(&cell->walls);

        // Increment the coordinate
        // Yes this is painful, but it handles n dimensions
        coords[maze->dims - 1]++;
        int overflow = coords[maze->dims - 1] >= maze->dims_array[maze->dims - 1];
        for (long d = maze->dims - 2; d >= 0 && overflow; d--) {
            coords[d + 1] =  0;
            coords[d]++;
            overflow = coords[d] >= maze->dims_array[d];
        }
        if (overflow) break;
    }
    free(coords);

}

// generate a 3d maze with 6-connected neighbors
// i.e. any 2 cells who's coords differ by 1 and only 1 in 1 and only 1 dimension are neighbors
struct maze* gen_maze_3d_6(unsigned long rows, unsigned long cols, unsigned long depth, unsigned long limit, void (*write_step)(const struct maze*, const struct cell*, unsigned int)) {
    // Init a grid
    unsigned long dims_array[] = {rows, cols, depth};
    struct maze* out = alloc_maze(3, dims_array);

    // Link cells with walls
    link_neighs(out);

    // Create a maze starting from a random cell
    // TODO save and return this? something something the maze is a tree?
    struct cell* start = ((struct cell****)out->maze)
        [(unsigned long)rand() % out->dims_array[0]]
        [(unsigned long)rand() % out->dims_array[1]]
        [(unsigned long)rand() % out->dims_array[2]];
    gen_maze(start, limit, out, write_step);
    return out;
}


// generate a 2d maze with 4-connected neighbors
// i.e. any 2 cells who's coords differ by 1 and only 1 in 1 and only 1 dimension are neighbors
struct maze* gen_maze_4(unsigned long rows, unsigned long cols, void (*relocate)(struct cell*), unsigned long limit, void (*write_step)(const struct maze*, const struct cell*, unsigned int)) {
    // Init a grid
    unsigned long dims_array[] = {rows, cols};
    struct maze* out = alloc_maze(2, dims_array);

    // Link cells with walls
    link_neighs(out);

    // Assign row and col for printing
    // TODO
    unsigned long coords[2];
    for (coords[0] = 0; coords[0] < rows; coords[0]++) {
        for (coords[1] = 0; coords[1] < cols; coords[1]++) {
            struct cell* new = get_cell(out, coords);
            new->row = new->coords[0];
            new->col = new->coords[1];
            relocate(new);// Ugh TODO
        }
    }

    // Create a maze starting from a random cell
    // TODO save and return this? something something the maze is a tree?
    struct cell* start = out->maze
        [(unsigned long)rand() % out->dims_array[0]]
        [(unsigned long)rand() % out->dims_array[1]];
    gen_maze(start, limit, out, write_step);
    return out;
}

/**
 * Build a maze from a given starting node.
 *
 * This modifies the nodes in place, solving for a solution and marking walls
 * as paths via depth first search.
 *
 * This function has no dependencies on the number of neighbors a node has, so
 * mazes of arbitrary connectedness or size should be generatable.
 */
void gen_maze(struct cell* node, unsigned long limit, struct maze* maze, void (*write_step)(const struct maze*, const struct cell*, unsigned int)) {
    unsigned int step = 0;
    if (write_step) write_step(maze, NULL, step++);
    unsigned long len = 0; // TODO describe this
    // Create a maze
    stack_t stack = new_stack();

    // mark visited
    node->visited = 1;
    // push to stack
    intmax_t stack_size = 0;
    stack_push(stack, node); stack_size++;
    while (stack_peek(stack)) {
        if (limit && len++ >= limit) break;
        // pop
        node = (struct cell*) stack_pop(stack); stack_size--;
        if (write_step) write_step(maze, node, step++);
        // pick an unvisited neighbor
        struct list_node* wall = node->walls.start;
        for (; wall != NULL;) {
            struct list_node* next = wall->next;
            if (!wall->cell->visited) {
                stack_push(stack, node); stack_size++;
                // remove the wall
                list_remove(&node->walls, wall);
                list_remove_data(&wall->cell->walls, node); // This will will leave us with a tree of paths
                wall->next = node->paths.start;
                node->paths.start = wall;
                if (node->paths.end == NULL) node->paths.end = wall;
                // mark as visited and push to stack
                wall->cell->visited = 1;
                stack_push(stack, wall->cell); stack_size++;
                break;
            }
            wall = next;
        }
    }

    if (write_step) write_step(maze, NULL, step++);

    stack_deallocate(stack);
}


/* Recursively free each dimension of a maze's cells
 * Args:
 * - dims: the length of the dims_array
 * - dims_array: array of lenths of each dimension
 * - target: the stuff to be freed
 */
void free_maze_cells(unsigned dims, unsigned long* dims_array, struct cell** target) {
    if (dims == 0) {
        struct cell* node = (struct cell*)target;
        list_deallocate(&node->walls);
        list_deallocate(&node->paths);
        free(node->coords);
        free(node);
    } else {
        for (unsigned long i = 0; i < dims_array[0]; i++) {
            free_maze_cells(dims-1, dims_array+1, (struct cell**)target[i]);
        }
        free(target);
    }
}

void clean_maze(struct maze* input) {
    free_maze_cells(input->dims, input->dims_array, (struct cell**)input->maze);
    free(input->dims_array);
    free(input);
}

