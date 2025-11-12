#include <stddef.h>
#include <stdlib.h>

#include "list.h"

struct list *list_sort(struct list *list)
{
    if (!list || !list->next)
    {
        return list;
    }
    int swapped;
    struct list *ptr;
    struct list *last = NULL;
    do
    {
        swapped = 0;
        ptr = list;
        while (ptr->next != last)
        {
            if (ptr->data > ptr->next->data)
            {
                int tmp = ptr->data;
                ptr->data = ptr->next->data;
                ptr->next->data = tmp;
                swapped = 1;
            }
            ptr = ptr->next;
        }
        last = ptr;
    } while (swapped);
    return list;
}

struct list *list_reverse(struct list *list)
{
    struct list *prev = NULL;
    struct list *current = list;
    struct list *next = NULL;
    while (current)
    {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    return prev;
}

struct list *list_split(struct list *list, size_t index)
{
    if (!list || index == 0)
    {
        return NULL;
    }
    struct list *current = list;
    size_t i = 0;
    while (current->next != NULL && i < index)
    {
        current = current->next;
        i++;
    }
    struct list *second_part = current->next;
    current->next = NULL;
    return second_part;
}
