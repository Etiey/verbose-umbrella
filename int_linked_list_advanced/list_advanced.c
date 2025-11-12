#include <stddef.h>
#include <stdlib.h>

#include "list.h"

struct list *list_insert(struct list *list, int value, size_t index)
{
    struct list *new_node = malloc(sizeof(struct list));
    if (!new_node)
    {
        return NULL;
    }
    new_node->data = value;
    if (index == 0)
    {
        new_node->next = list;
        return new_node;
    }
    struct list *current = list;
    size_t i = 0;
    while (current && i < index - 1)
    {
        current = current->next;
        i++;
    }
    if (!current)
    {
        free(new_node);
        return list_append(list, value);
    }
    new_node->next = current->next;
    current->next = new_node;
    return list;
}

struct list *list_remove(struct list *list, size_t index)
{
    if (!list)
    {
        return NULL;
    }
    if (index == 0)
    {
        struct list *new_head = list->next;
        free(list);
        return new_head;
    }
    struct list *current = list;
    size_t i = 0;
    while (current->next && i < index - 1)
    {
        current = current->next;
        i++;
    }
    if (!current->next)
    {
        return list;
    }
    struct list *to_delete = current->next;
    current->next = to_delete->next;
    free(to_delete);
    return list;
}

int list_find(struct list *list, int value)
{
    struct list *current = list;
    int position = 0;
    while (current)
    {
        if (current->data == value)
            return position;

        current = current->next;
        position++;
    }

    return -1;
}

struct list *list_concat(struct list *list, struct list *list2)
{
    if (!list)
    {
        return list2;
    }
    if (!list2)
    {
        return list;
    }
    struct list *current = list;
    while (current->next)
    {
        current = current->next;
    }
    current->next = list2;
    return list;
}
