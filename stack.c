#include <stdlib.h> // malloc(), calloc, free(), NULL

/* A stack node */
typedef struct stack {
    void* data;         // data stored in this node
    struct stack* next; // next node in the stack. NULL if this is the last one
}** stack_t;

stack_t new_stack(void) {
    stack_t stack = calloc(sizeof(struct stack), 1);
    *stack = NULL;
    return stack;
}

void stack_deallocate(stack_t stack) {
    while (*stack != NULL) {
        struct stack* current = *stack;
        *stack = current->next;
        free(current);
    }
    free(stack);
}

// if the stack is empty, returns a null pointer
void* stack_pop(stack_t stack) {
    if (*stack != NULL) {
        struct stack* old = *stack;
        void* data = old->data;
        *stack = old->next;
        free(old);
        return data;
    }

    return NULL;
}

void stack_push(stack_t stack, void* data) {
    struct stack* next = malloc(sizeof(struct stack));
    next->data = data;
    next->next = *stack;
    *stack = next;
}

// Return 1 if there is something on the stack, 0 otherwise
int stack_peek(stack_t stack) {
    if (*stack != NULL) {
        return 1;
    } else {
        return 0;
    }
}
