#include "stack.h"
#include <stdio.h>

int main(void) {

    stack_t stack = new_stack();
    stack_push(stack, (void*)"1");
    stack_push(stack, (void*)"2");
    stack_push(stack, (void*)"3");
    while (stack_peek(stack)) {
        printf("stack: %s\n", (char*)stack_pop(stack));
    }
    stack_deallocate(stack);

    return 0;
}
