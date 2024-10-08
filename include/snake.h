#ifndef PICOS_SNAKE_H
#define PICOS_SNAKE_H

#include <array>
#include <string_view>
#include <stdio.h>

#include "textbox.h"
#include "util.h"

// Static configuration

// Width of the field, in characters
constexpr static const int FIELD_WIDTH = 40;

// Height of the field, in characters
constexpr static const int FIELD_HEIGHT = 40;

constexpr static const auto FRAME_LATENCY = 250; // ms
constexpr static const bool PASS_THROUGH_WALLS = true;
constexpr static const int FRUIT_VALUE = 10;
constexpr static const int FRUIT_COUNT = 10;

constexpr static const char WALL_CHAR = '#' - 0x20;
constexpr static const char SPACE_CHAR = ' ' - 0x20;
constexpr static const char FRUIT_CHAR = 'F' - 0x20;
constexpr static const char SNAKE_HEAD_CHAR = 'O' - 0x20;
constexpr static const char SNAKE_BODY_CHAR = '@' - 0x20;


enum class Direction { UP, DOWN, LEFT, RIGHT };

// Get a number in [min, max)
int getRandom(int min, int max) {
    double r = rand();
    r /= RAND_MAX;
    r *= (max - min);
    r += min;

    return (int)r;
} // getRandom

class SnakeSection {
  public:
    Position position;
    Direction direction;

    // Move this section in the direction it is pointing
    void applyDirection() {
        switch (direction) {
        case Direction::UP:
            position.y--;
            break;
        case Direction::DOWN:
            position.y++;
            break;
        case Direction::LEFT:
            position.x--;
            break;
        case Direction::RIGHT:
            position.x++;
            break;
        }
    } // applyDirection

    // Rotate right a certain number of times
    void turn(int quarters = 1) {
        quarters %= 4;

        for (int i = 0; i < quarters; i++) {
            switch (direction) {
            case Direction::UP:
                direction = Direction::RIGHT;
                break;
            case Direction::DOWN:
                direction = Direction::LEFT;
                break;
            case Direction::LEFT:
                direction = Direction::UP;
                break;
            case Direction::RIGHT:
                direction = Direction::DOWN;
                break;
            }
        }
    } // turn
};

class Field {
public:
    // Buffer storing visual state of the field
    FixedTextWindow<FIELD_WIDTH, FIELD_HEIGHT>& fieldText;

    char wallCopy[FIELD_WIDTH * FIELD_HEIGHT];

    Field(FixedTextWindow<FIELD_WIDTH, FIELD_HEIGHT>& fieldText_): fieldText(fieldText_) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
            for (int y = 0; y < FIELD_HEIGHT; y++) {
                wallCopy[(y * FIELD_WIDTH) + x] = isWall({ x, y }) ? WALL_CHAR : SPACE_CHAR;
            }
        }
    }

    bool isWall(Position position) {
        return position.x == 0 || position.x == (FIELD_WIDTH - 1) ||
               position.y == 0 || position.y == (FIELD_HEIGHT - 1);
    }

    // Remove all characters added to the field, leaving only walls
    void reset() {
        fieldText.copyFrom(wallCopy);
    }
};

class Fruit {
  public:
    Position position;

    void place(Position _pos) {
        position = _pos;
    } // place

    void draw(Field& field) {
        field.fieldText.at(position) = FRUIT_CHAR;
    } // draw
};

class Snake {
    bool dead = false;

  public:
    std::vector<SnakeSection> sections;
    int sectionsToAdd = 0;

    void place(Position position, Direction direction) {
        sections.push_back({ position, direction });
    } // place

    void draw(Field& field) {
        for (int i = sections.size() - 1; i >= 0; i--) {
            SnakeSection& section = sections[i];
            field.fieldText.at(section.position) = i == 0 ? SNAKE_HEAD_CHAR : SNAKE_BODY_CHAR;
        }
    } // draw

    SnakeSection& getHead() {
        return sections[0];
    } // getHead

    void addSection() {
        sectionsToAdd++;
    } // addSection

    void die() {
        dead = true;
    } // die

    bool isDead() {
        return dead;
    } // isDead
};

class SnakeGame : public FixedTextWindow<FIELD_WIDTH, FIELD_HEIGHT> {
public:
    int score = 0;

    int lastRunFrame = 0;

    Field field;
    Snake snake;
    std::array<Fruit, FRUIT_COUNT> fruits;

    SnakeGame() : FixedTextWindow(0, 1), field(*this) {
        snake.place(placeObject(), Direction::RIGHT);

        for (int i = 0; i < (int)fruits.size(); i++) {
            fruits[i].place(placeObject());
        }
    }

    void tickGame() {
        tickSnake();

        field.reset();

        // Draw the objects on the field
        for (int i = 0; i < (int)fruits.size(); i++) {
            fruits[i].draw(field);
        }

        snake.draw(field);
    } // tick

    void tickSnake() {
        // Take tail by value so that it can be appended after all pieces have moved
        SnakeSection tail = snake.sections[snake.sections.size() - 1];

        moveSnakeBody();

        if (snake.sectionsToAdd > 0) {
            snake.sections.push_back(tail);
            snake.sectionsToAdd--;
        }

        moveSnakeHead();
    } // tickSnake

    void moveSnakeBody() {
        // Move each section to the position of the section in front of it
        for (int i = (int)snake.sections.size() - 1; i > 0; i--) {
            SnakeSection& section = snake.sections[i];

            section.position = snake.sections[i - 1].position;
            section.direction = snake.sections[i - 1].direction;
        }
    } // moveSnakeBody

    void moveSnakeHead() {
        SnakeSection& head = snake.getHead();

        head.applyDirection();

        if (field.isWall(head.position)) {
            if (PASS_THROUGH_WALLS) {
                // Pass through to the other side
                if (head.position.x == FIELD_WIDTH - 1) {
                    head.position.x = 1;
                } else if (head.position.x == 0) {
                    head.position.x = FIELD_WIDTH - 2;
                } else if (head.position.y == FIELD_HEIGHT - 1) {
                    head.position.y = 1;
                } else if (head.position.y == 0) {
                    head.position.y = FIELD_HEIGHT - 2;
                }
            } else {
                // Die when a segment of the snake attempts to inhabit a wall space
                snake.die();
            }
        }

        if (intersectsWithFruit(head.position)) {
            consumeFruit(head.position);
        }

        if (didSnakeCollide()) {
            snake.die();
        }
    } // moveSnakeHead

    void consumeFruit(Position position) {
        // Figure out which fruit was eaten
        Fruit* fruit = &fruits[0];
        for (int i = 0; i < (int)fruits.size(); i++) {
            if (fruits[i].position == position) {
                fruit = &fruits[i];
                break;
            }
        }

        score += FRUIT_VALUE;

        snake.addSection();

        fruit->position = { 0, 0 };
        fruit->position = placeObject();
    } // consumeFruit

    bool intersectsWithSnake(Position position) {
        for (int i = 0; i < (int)snake.sections.size(); i++) {
            SnakeSection segment = snake.sections[i];
            if (position == segment.position) {
                return true;
            }
        }
        return false;
    } // intersectsWithSnake

    bool intersectsWithFruit(Position position) {
        for (int i = 0; i < (int)fruits.size(); i++) {
            if (position == fruits[i].position) {
                return true;
            }
        }
        return false;
    } // intersectsWithFruit

    bool intersectsWithObject(Position position) {
        return intersectsWithSnake(position) || field.isWall(position) || intersectsWithFruit(position);
    } // intersectsWithObject

    bool didSnakeCollide() {
        // A snake dies when any of its segments are in walls or each other
        for (int i = 0; i < (int)snake.sections.size(); i++) {
            SnakeSection segment = snake.sections[i];

            if (field.isWall(segment.position) && !PASS_THROUGH_WALLS) {
                return true;
            }

            if (i != 0 && segment.position == snake.getHead().position) {
                return true;
            }
        }

        return false;
    } // didSnakeCollide

    // Try to find a safe place to put something
    Position placeObject() {
        Position ret;
        do {
            ret = { getRandom(2, FIELD_WIDTH - 2), getRandom(2, FIELD_HEIGHT - 2) };
        } while (intersectsWithObject(ret));

        return ret;
    } // placeObject

    void getInput() {
        if (buttons[0].justPressed()) {
            snake.getHead().direction = Direction::LEFT;
        }
        else if (buttons[1].justPressed()) {
            static bool s = false;
            s = !s;
            snake.getHead().turn(3);
        }
        else if (buttons[2].justPressed()) {
            snake.getHead().turn(1);
        }
    } // getInput

    void tick(uint16_t frameNum) {
        buttons[0].update();
        buttons[1].update();
        buttons[2].update();

        if (frameNum % 10 == 0 && lastRunFrame != frameNum) {
            lastRunFrame = frameNum;

            getInput();

            tickGame();
        }
    }

    Window::Type getType() {
        return Window::Type::SnakeGame;
    }
};

SnakeGame* snakeInst;

constexpr static const int SCORE_WIDTH = 28;
constexpr static const int SCORE_HEIGHT = 8;

class SnakeScore : public FixedTextWindow<SCORE_WIDTH, SCORE_HEIGHT> {
    public:
    char blankBuf[SCORE_WIDTH * SCORE_HEIGHT] = {};

    SnakeScore(): FixedTextWindow(snakeInst->x_max + FONT_WIDTH, 30) {
        for (int i = 0; i < SCORE_WIDTH * SCORE_HEIGHT; i++) {
            blankBuf[i] = 0;
        }
    }

    void convAscii(char* buf, int len) {
        for (char* i = buf; i < buf + len; i++) {
            *i = fontConv[*i];
        }
    }

    void tick(uint16_t frameNum) {
        if (frameNum % 10 == 0) {
            copyFrom(blankBuf);

            int len = snprintf(&at({0, 0}), SCORE_WIDTH, "Snake!    By Nicholas Karr");
            CHECK(len >= 0);
            convAscii(&at({0, 0}), len);

            len = snprintf(&at({0, 1}), SCORE_WIDTH, "Score: %d", snakeInst->score);
            CHECK(len >= 0);
            convAscii(&at({0, 1}), len);

            absolute_time_t time = get_absolute_time();
            uint32_t ms = to_ms_since_boot(time);
            len = snprintf(&at({0, 2}), SCORE_WIDTH, "Time: %d", int(ms / 1000));
            CHECK(len >= 0);
            convAscii(&at({0, 2}), len);

            len = snprintf(&at({0, 3}), SCORE_WIDTH, "Controls: Menu  Left  Right");
            CHECK(len >= 0);
            convAscii(&at({0, 3}), len);
        }
    }

    Window::Type getType() {
        return Window::Type::SnakeScore;
    }
};

SnakeScore* snakeScoreInst;

void initSnakeGame() {
    srand(0);

    snakeInst = new SnakeGame();

    snakeScoreInst = new SnakeScore();
}

#endif // ifndef PICOS_SNAKE_H