#include <new>

template <typename DataT, int size>
struct RingBuffer {
    DataT buf[size];
    DataT* begin;
    DataT* end;

public:
    RingBuffer() {
        reset();
        for (int i = 0; i < size; i++) {
            new(&buf[i]) DataT();
        }
    }

    void reset() {
        begin = end = buf;
    }

    DataT* alloc() {
        if (end >= buf + size) {
            return end = begin;
        }
        else {
            return end++;
        }
    }
};