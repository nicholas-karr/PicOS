#ifndef PICOS_MEM_H
#define PICOS_MEM_H

#include <cstring>

// Non-overlapping templated fast memcpy
template <size_t num>
void memcpyFast(void* __restrict__ dest, const void* __restrict__ src) {
    memcpy(dest, src, num);
    return;
}

#endif // ifndef PICOS_MEM_H