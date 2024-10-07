extern "C" {
    void memcpyFast8(void* dest, void* src, size_t num);
    void memcpyFast128(void* dest, void* src, size_t num);
}

// Non-overlapping fast memcpy
template <size_t num>
void memcpyFast(void* __restrict__ dest, void* __restrict__ src) {
    if constexpr (num % 16 == 0) {
        memcpyFast128(dest, src, num);
    }
    else {
        memcpyFast8(dest, src, num);
    }
}
