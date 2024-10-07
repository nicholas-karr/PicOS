#include <iostream> // cout
#include <vector> // vector
#include <array> // array
#include <thread> // sleep_for
#include <ctime> // time() for seeding RNG
#include <chrono>

#include "textbox.h"

// Static configuration
constexpr static const int FIELD_WIDTH = 40;
constexpr static const int FIELD_HEIGHT = 40;

constexpr static const auto FRAME_LATENCY = std::chrono::milliseconds(250);
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

class Position {
  public:
    int x, y;

    bool operator==(const Position& rhs) const noexcept {
        return this->x == rhs.x && this->y == rhs.y;
    } // operator==
};

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
    char fieldText[(FIELD_WIDTH + 1) * (FIELD_HEIGHT + 1)] = {};

    // Access the text character at a position on the field
    char& at(Position position) {
        return fieldText[(position.y * FIELD_WIDTH) + position.x];
    } // at

    bool isWall(Position position) {
        return position.x == 0 || position.x == FIELD_WIDTH ||
               position.y == 0 || position.y == FIELD_HEIGHT;
    } // isWall

    // Remove all characters added to the field, leaving only walls
    void reset() {
        for (int x = 0; x < FIELD_WIDTH + 1; x++) {
            for (int y = 0; y < FIELD_HEIGHT + 1; y++) {
                at({ x, y }) = isWall({ x, y }) ? WALL_CHAR : SPACE_CHAR;
            }
        }
    } // reset

    // Print the field to the console
    void print() {
        for (int y = 0; y < FIELD_HEIGHT + 1; y++) {
            for (int x = 0; x < FIELD_WIDTH + 1; x++) {
                std::cout << at({ x, y });
            }
            std::cout << '\n';
        }
    } // print
};

class Fruit {
  public:
    Position position;

    void place(Position _pos) {
        position = _pos;
    } // place

    void draw(Field& field) {
        field.at(position) = FRUIT_CHAR;
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
            field.at(section.position) = i == 0 ? SNAKE_HEAD_CHAR : SNAKE_BODY_CHAR;
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
        while (true) {
            std::cout << "";
        }
    } // die

    bool isDead() {
        return dead;
    } // isDead
};

class SnakeGame : public Window {
    int score = 0;
    int lastRunFrame = 0;

    Field field;
    Snake snake;
    std::array<Fruit, FRUIT_COUNT> fruits;

    void tickGame(double deltaTime) {
        tickSnake(deltaTime);

        field.reset();

        // Draw the objects on the field
        for (int i = 0; i < (int)fruits.size(); i++) {
            fruits[i].draw(field);
        }

        snake.draw(field);

        // Output the field
        //field.print();
        //std::cout << "Score: " << score << '\n';
    } // tick

    void tickSnake(double /*deltaTime*/) {
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
                if (head.position.x == FIELD_WIDTH) {
                    head.position.x = 1;
                } else if (head.position.x == 0) {
                    head.position.x = FIELD_WIDTH - 1;
                } else if (head.position.y == FIELD_HEIGHT) {
                    head.position.y = 1;
                } else if (head.position.y == 0) {
                    head.position.y = FIELD_HEIGHT - 1;
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
            ret = { getRandom(1, FIELD_WIDTH - 1), getRandom(1, FIELD_HEIGHT - 1) };
        } while (intersectsWithObject(ret));

        return ret;
    } // placeObject

    void getInput() {
        std::cout << "History " << (int)buttons[0].history_ << ' ' << (int)buttons[1].history_ << ' ' << (int)buttons[2].history_ << "\r\n";

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

    void clearConsole() {
        for (int i = 0; i < 50; i++) {
            std::cout << '\n';
        }
    } // clearConsole

  public:
    SnakeGame() {
        memset(field.fieldText, ' ' - 0x20, sizeof(field.fieldText));

        x = 0;
        x_max = (FIELD_WIDTH + 2) * 4;
        //x_max -= 4;
        y = 0;
        y_max = (FIELD_HEIGHT + 2) * FONT_HEIGHT;
        //y_max -= 4;

        snake.place(placeObject(), Direction::RIGHT);

        for (int i = 0; i < (int)fruits.size(); i++) {
            fruits[i].place(placeObject());
        }
    } // SnakeGame()

    void run() {
        while (!snake.isDead()) {
            clearConsole();

            getInput();

            tick((double)FRAME_LATENCY.count());

            //std::this_thread::sleep_for(FRAME_LATENCY);
            sleep_ms(FRAME_LATENCY.count());
        }

        std::cout << "You died! Final Score: " << score;
    } // run

    void tick(uint16_t frameNum) {
        buttons[0].update();
        buttons[1].update();
        buttons[2].update();

        if (frameNum % 10 == 0 && lastRunFrame != frameNum) {
            lastRunFrame = frameNum;

            getInput();

            tickGame(0);
        }
    }

    std::string_view lineAt(int y) {
        //return std::string_view(field.fieldText + y * FIELD_WIDTH);
        //return windows[1]->lineAt(y);

        y -= this->y;
        int index = y / FONT_HEIGHT;
        if (index < 0 || index > FIELD_HEIGHT + 1) {
            return nullptr;
        }
        //index %= 10;
        return std::string_view(field.fieldText + index * (FIELD_WIDTH), FIELD_WIDTH);// + y * (FIELD_WIDTH + 1), 5);
        //return windows[1]->lineAt(y);
    }

    Window::Type getType() {
        return Window::Type::SnakeGame;
    }
};

SnakeGame* snakeInst;

void initSnakeGame() {
    srand((unsigned int)std::time(NULL));

    snakeInst = new SnakeGame();
}