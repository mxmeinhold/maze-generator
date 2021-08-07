#include "stack.h"
#include "tree.h"
#include "generator.h"
#include <stdio.h>
#include <stdint.h> // intmax_t
#include <img.h>
#include <stdlib.h> // calloc(), srand()
#include <string.h> // strcmp()

void relocate(struct cell* c) {
    c->row = c->row * 2 + 1;
    c->col = c->col * 2 + 1;
}

intmax_t strcompare(const void* self, const void* other) {
    return (intmax_t) strcmp((void*) self, (void*) other);
}

int main(void) {
    srand(0xf00f);

    stack_t stack = new_stack();
    stack_push(stack, (void*)"1");
    stack_push(stack, (void*)"2");
    stack_push(stack, (void*)"3");
    while (stack_peek(stack)) {
        printf("stack: %s\n", (char*)stack_pop(stack));
    }
    stack_deallocate(stack);

    tree_t tree = new_tree(&strcompare);
    tree_add(tree, (void*)"2");
    tree_add(tree, (void*)"1");
    tree_add(tree, (void*)"3");
    char* current;
    while ((current = tree_pop(tree)) != NULL) {
        printf("tree: %s\n", current);
    }
    tree_deallocate(tree);


    short rows = 50;
    short cols = 50;

    struct maze* maze = gen_maze(rows, cols, &relocate);

    struct img img;
    img.width = 2 * rows + 1;
    img.height = 2 * cols + 1;
    img.rows = calloc(sizeof(struct pixel*), (size_t)img.height);
    for (int r = 0; r < img.height; r++) {
        img.rows[r] = calloc(sizeof(struct pixel), (size_t)img.width);
        for (int c = 0; c < img.width; c++) {
            // It might be a wall. More on that later.
            img.rows[r][c].red = 0;
            img.rows[r][c].green = 0;
            img.rows[r][c].blue = 0;
        }
    }

    for (unsigned short r = 0; r < maze->rows; r++) {
        for (unsigned short c = 0; c < maze->cols; c++) {
            struct cell* cell = maze->maze[r][c];
            img.rows[cell->row][cell->col].red = 255;
            img.rows[cell->row][cell->col].green = 255;
            img.rows[cell->row][cell->col].blue = 255;
            for (struct list_node* path = cell->paths.start; path != NULL; path = path->next) {
                int row = path->cell->row + (cell->row - path->cell->row) / 2;
                int col = path->cell->col + (cell->col - path->cell->col) / 2;
                img.rows[row][col].red = 255;
                img.rows[row][col].green = 255;
                img.rows[row][col].blue = 255;
            }
        }
    }
    writepng("maze.png", &img);
    clean_maze(maze);

    // Free the image
    for (int r = 0; r < img.height; r++) free(img.rows[r]);
    free(img.rows);

    return 0;
}
