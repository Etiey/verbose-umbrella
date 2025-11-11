#include <stddef.h>
#include <stdint.h>

/*
** Hash the key using FNV-1a 32 bits hash algorithm.
*/
size_t hash(const char *key)
{
    if (!key)
        return 0;

    uint32_t hash = 2166136261; // FNV offset basis
    uint32_t prime = 16777619; // FNV prime

    while (*key)
    {
        hash ^= *key;
        hash *= prime;
        key++;
    }

    return hash;
}
