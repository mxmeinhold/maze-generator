#include "stack.h"
#include "tree.h"
#include <stdio.h>
#include <stdint.h> // intmax_t
#include <string.h> // strcmp()

intmax_t strcompare(const void* self, const void* other) {
    return (intmax_t) strcmp((void*) self, (void*) other);
}

int main(void) {

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

    return 0;
}
