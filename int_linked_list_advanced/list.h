#ifndef LIST_H
#define LIST_H

#include <stddef.h>

struct list
{
    int data;
    struct list *next;
};

struct list *list_prepend(struct list *list, int value);

size_t list_length(struct list *list);

void list_print(struct list *list);

void list_destroy(struct list *list);

struct list *list_append(struct list *list, int value);

struct list *list_insert(struct list *list, int value, size_t index);

struct list *list_remove(struct list *list, size_t index);

int list_find(struct list *list, int value);

struct list *list_concat(struct list *list, struct list *list2);

struct list *list_sort(struct list *list);

struct list *list_reverse(struct list *list);

struct list *list_split(struct list *list, size_t index);

#endif /* ! LIST_H */
