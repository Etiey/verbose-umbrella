#include <stddef.h>

__attribute__((visibility("default"))) void *malloc(size_t size)
{
    return NULL;
}

__attribute__((visibility("default"))) void free(void *ptr)
{}

__attribute__((visibility("default"))) void *realloc(void *ptr, size_t size)
{
    return NULL;
}

__attribute__((visibility("default"))) void *calloc(size_t nmemb, size_t size)
{
    return NULL;
}
