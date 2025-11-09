#include "alignment.h"

size_t align(size_t size)
{
    size_t alignment = sizeof(long double);
    size_t max_val = 0;
    max_val = ~max_val;
    if (size == 0)
    {
        return 0;
    }
    if (size > max_val - (alignment - 1))
    {
        return 0;
    }
    if (size % alignment == 0)
    {
        return size;
    }
    size_t t = size % alignment;
    size_t aligned = size + (alignment - t);
    return aligned;
}
