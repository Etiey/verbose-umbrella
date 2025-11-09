#include "beware_overflow.h"

#include <stddef.h>

void *beware_overflow(void *ptr, size_t nmemb, size_t size)
{
    if (nmemb == 0 || size == 0)
    {
        return ptr;
    }
    size_t result = nmemb * size;
    if (result / nmemb != size)
    {
        return NULL;
    }
    char *byte_ptr = ptr;
    return byte_ptr + result;
}
