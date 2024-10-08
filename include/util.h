#ifndef PICOS_UTIL_H
#define PICOS_UTIL_H

class Position {
  public:
    int x, y;

    bool operator==(const Position& rhs) const noexcept {
        return this->x == rhs.x && this->y == rhs.y;
    }
};

// Verify the success of a boolean
inline void checkImpl(bool val, const char* file, int line)
{
    if (!val) {
        while (true) {
            printf("Function call in %s on line %d failed\n", file, line);
            sleep_ms(1000);
        }
    }
}

#ifdef DO_CHECKS
#define CHECK(val) (checkImpl(val, __FILE__, __LINE__))
#else
#define CHECK(val)
#endif

#endif // ifndef PICOS_UTIL_H