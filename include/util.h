#ifndef PICOS_UTIL_H
#define PICOS_UTIL_H

class Position {
  public:
    int x, y;

    bool operator==(const Position& rhs) const noexcept {
        return this->x == rhs.x && this->y == rhs.y;
    }
};

#endif // ifndef PICOS_UTIL_H