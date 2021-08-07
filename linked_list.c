#include "generator.h"

void list_push(struct linked_list* list, struct cell* cell) {
    struct list_node* new = malloc(sizeof(struct list_node));
    new->cell = cell;
    new->next = NULL;
    new->prev = list->end;
    if (list->end != NULL) list->end->next = new;
    if (list->start == NULL) list->start = new;
    list->end = new;
}

void list_remove(struct linked_list* list, struct list_node* node) {
    if (list->start == node) list->start = node->next;
    if (list->end == node) list->end = node->prev;
    if (node->prev != NULL) node->prev->next = node->next;
    if (node->next != NULL) node->next->prev = node->prev;
}

void list_remove_data(struct linked_list* list, struct cell* cell) {
    for (struct list_node* node = list->start; node != NULL; node = node->next) {
        if (node->cell == cell) {
            list_remove(list, node);
            free(node);
            break;
        }
    }
}

void list_deallocate(struct linked_list* list) {
    struct list_node* tmp;
    for (struct list_node* node = list->start; node != NULL;) {
        tmp = node->next;
        free(node);
        node = tmp;
    }
}
