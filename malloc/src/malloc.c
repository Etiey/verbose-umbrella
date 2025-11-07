#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_SIZE 4096

size_t align(size_t size)
{
    return (size + 15) & ~15;
}

void *beware_overflow(void *ptr, size_t nmemb, size_t size)
{
    size_t res;
    if (__builtin_mul_overflow(nmemb, size, &res))
    {
        return NULL;
    }
    return ptr ? (char *)ptr + res : (void *)(uintptr_t)1;
}

void *page_begin(void *ptr, size_t page_size)
{
    return (void *)((uintptr_t)ptr & ~(page_size - 1));
}

struct recycler
{
    size_t block_size;
    size_t capacity;
    void *chunk;
    void *free;
};

struct free_list
{
    struct free_list *next;
};

struct recycler *recycler_create(size_t block_size, size_t total_size)
{
    if (block_size < sizeof(struct free_list))
    {
        block_size = sizeof(struct free_list);
    }
    struct recycler *r =
        mmap(NULL, sizeof(struct recycler), PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (r == MAP_FAILED)
    {
        return NULL;
    }
    r->chunk = mmap(NULL, total_size, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (r->chunk == MAP_FAILED)
    {
        munmap(r, sizeof(struct recycler));
        return NULL;
    }
    r->block_size = block_size;
    r->capacity = total_size / block_size;
    r->free = r->chunk;
    struct free_list *curr = r->free;
    for (size_t i = 0; i < r->capacity - 1; ++i)
    {
        curr->next = (struct free_list *)((char *)curr + block_size);
        curr = curr->next;
    }
    curr->next = NULL;
    return r;
}

void *recycler_allocate(struct recycler *r)
{
    if (!r || !r->free)
    {
        return NULL;
    }
    struct free_list *node = r->free;
    r->free = node->next;
    return node;
}

void recycler_free(struct recycler *r, void *block)
{
    if (!r || !block)
    {
        return;
    }
    struct free_list *node = block;
    node->next = r->free;
    r->free = node;
}

static pthread_mutex_t g_malloc_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct recycler *g_recycler = NULL;

static void init_allocator_if_needed(void)
{
    if (!g_recycler)
    {
        g_recycler = recycler_create(16, 1024 * 1024);
    }
}

__attribute__((visibility("default"))) void *malloc(size_t size)
{
    if (size == 0)
    {
        return NULL;
    }
    pthread_mutex_lock(&g_malloc_mutex);
    init_allocator_if_needed();
    size_t aligned_size = align(size);
    void *ptr = NULL;
    if (g_recycler && aligned_size <= g_recycler->block_size)
    {
        ptr = recycler_allocate(g_recycler);
    }
    if (!ptr)
    {
        size_t total_size = align(aligned_size + sizeof(size_t));
        ptr = mmap(NULL, total_size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED)
        {
            pthread_mutex_unlock(&g_malloc_mutex);
            errno = ENOMEM;
            return NULL;
        }
        *(size_t *)ptr = total_size;
        ptr = (char *)ptr + sizeof(size_t);
    }
    pthread_mutex_unlock(&g_malloc_mutex);
    return ptr;
}

__attribute__((visibility("default"))) void free(void *ptr)
{
    if (!ptr)
    {
        return;
    }
    pthread_mutex_lock(&g_malloc_mutex);
    if (g_recycler && (char *)ptr >= (char *)g_recycler->chunk
        && (char *)ptr < (char *)g_recycler->chunk
                + (g_recycler->capacity * g_recycler->block_size))
    {
        recycler_free(g_recycler, ptr);
    }
    else
    {
        size_t *size_ptr = (size_t *)((char *)ptr - sizeof(size_t));
        munmap(size_ptr, *size_ptr);
    }
    pthread_mutex_unlock(&g_malloc_mutex);
}

__attribute__((visibility("default"))) void *calloc(size_t nmemb, size_t size)
{
    if (beware_overflow(NULL, nmemb, size) == NULL)
    {
        errno = ENOMEM;
        return NULL;
    }
    size_t total_size = nmemb * size;
    void *ptr = malloc(total_size);
    if (ptr)
    {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

__attribute__((visibility("default"))) void *realloc(void *ptr, size_t size)
{
    if (!ptr)
    {
        return malloc(size);
    }
    if (size == 0)
    {
        free(ptr);
        return NULL;
    }
    void *new_ptr = malloc(size);
    if (!new_ptr)
    {
        return NULL;
    }
    memcpy(new_ptr, ptr, size);
    free(ptr);
    return new_ptr;
}
