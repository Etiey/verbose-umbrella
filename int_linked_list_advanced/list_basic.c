#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "list.h"

struct list *list_prepend(struct list *list, int value)
{
    struct list *bloc = malloc(sizeof(struct list));
    if (bloc == NULL)
    {
        return NULL;
    }

    bloc->data = value;
    bloc->next = list;

    return bloc;
}

size_t list_length(struct list *list)
{
    if (!list)
    {
        return 0;
    }
    return 1 + list_length(list->next);
}

void list_print(struct list *list)
{
    if (!list)
    {
        return;
    }
    while (list->next != NULL)
    {
        printf("%d ", list->data);
        list = list->next;
    }
    printf("%d\n", list->data);
}

void list_destroy(struct list *list)
{
    if (!list)
    {
        return;
    }
    list_destroy(list->next);
    free(list);
}

struct list *list_append(struct list *list, int value)
{
    struct list *new_node = malloc(sizeof(struct list));
    if (!new_node)
    {
        return NULL;
    }
    new_node->data = value;
    new_node->next = NULL;
    if (!list)
    {
        return new_node;
    }
    struct list *current = list;
    while (current->next)
    {
        current = current->next;
    }
    current->next = new_node;
    return list;
}
