extern "C" {
    void memcpyFast8(void* dest, void* src, size_t num);
    void memcpyFast16(void* dest, void* src, size_t num) {};
    void memcpyFast32(void* dest, void* src, size_t num) {};
    void memcpyFast64(void* dest, void* src, size_t num) {};
    void memcpyFast128(void* dest, void* src, size_t num);
    void memcpyFast256(void* dest, void* src, size_t num) {};
    void memcpyFast512(void* dest, void* src, size_t num) {};
}

// Non-overlapping fast memcpy
template <size_t num>
void memcpyFast(void* dest, void* src) {
    if constexpr (num % 16 == 0) {
        memcpyFast128(dest, src, num);
    }
    if constexpr (num % 8 == 0) {
        memcpyFast64(dest, src, num);
    }
    if constexpr (num % 4 == 0) {
        memcpyFast32(dest, src, num);
    }
    else if constexpr (num % 2 == 0) {
        memcpyFast16(dest, src, num);
    }
    else {
        memcpyFast8(dest, src, num);
    }
}
