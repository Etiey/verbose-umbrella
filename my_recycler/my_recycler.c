#include "my_recycler.h"

#include <stddef.h>
#include <stdlib.h>

/*4*/
struct recycler *recycler_create(size_t block_size, size_t total_size)
{
    if (block_size == 0 || total_size == 0)
    {
        return NULL;
    }
    if (block_size % sizeof(long double) != 0)
    {
        return NULL;
    }
    if (total_size % block_size != 0)
    {
        return NULL;
    }
    struct recycler *r = malloc(sizeof(struct recycler));
    if (!r)
    {
        return NULL;
    }
    r->chunk = calloc(1, total_size);
    if (!r->chunk)
    {
        free(r);
        return NULL;
    }
    r->block_size = block_size;
    r->capacity = total_size / block_size;
    r->free = r->chunk;

    struct free_list *current = r->chunk;
    char *base = r->chunk;
    for (size_t i = 0; i < r->capacity - 1; i++)
    {
        void *next_block = base + (i + 1) * block_size;
        current->next = next_block;
        current = next_block;
    }
    current->next = NULL;
    return r;
}

/*5*/
void recycler_destroy(struct recycler *r)
{
    if (!r)
    {
        return;
    }
    free(r->chunk);
    free(r);
}

/*6*/
void *recycler_allocate(struct recycler *r)
{
    if (!r || !r->free)
    {
        return NULL;
    }
    void *block = r->free;
    struct free_list *free_block = r->free;
    r->free = free_block->next;
    return block;
}

/*7*/
void recycler_free(struct recycler *r, void *block)
{
    if (!r || !block)
    {
        return;
    }
    struct free_list *freed_block = block;
    freed_block->next = r->free;
    r->free = block;
}
