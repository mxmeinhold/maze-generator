#ifndef MAZE_GEN_STACK_H
#define MAZE_GEN_STACK_H

typedef struct stack** stack_t;

/**
 * Instantiate and prep a new stack
 */
stack_t new_stack(void);

/**
 * Free all the stack resources
 * Note that this won't mangle or free the data that had been in the stack.
 */
void stack_deallocate(stack_t stack);

/**
 * Pop the first element off the stack.
 * If the stack is empty, this will return NULL.
 */
void* stack_pop(stack_t stack);

/**
 * Push a new element onto the stack.
 */
void stack_push(stack_t stack, void* data);

/**
 * Check if there's anything on the stack.
 * Returns 1 if there is, 0 otherwise.
 */
int stack_peek(stack_t stack);

#endif
