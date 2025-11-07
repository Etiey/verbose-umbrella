#include "page_begin.h"

void *page_begin(void *ptr, size_t page_size)
{
    union
    {
        void *p;
        size_t a;
    } u;
    u.p = ptr;
    u.a &= ~(page_size - 1);

    return u.p;
}
