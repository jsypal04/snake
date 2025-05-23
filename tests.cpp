#include "game.hpp"

enum result {
    PASSED,
    FAILED,
};

enum result snake_move_test_1() {
    Snake* snake = new Snake(0.0f, 0.0f);
    snake->move();
    
    Snake* correct_snake = new Snake(-0.03f, 0.0f);

    if (snake->equals(correct_snake)) {
        return PASSED;
    }

    return FAILED;
}

enum result snake_move_test_2() {
    Snake* correct_snake = new Snake(0.0f, 0.0f);
    correct_snake->squares.pop_back();
    Square* head = new Square(0.0f, 0.03f);
    correct_snake->squares.insert(correct_snake->squares.begin(), head);

    Snake* snake = new Snake(0.0f, 0.0f);
    snake->direction = 'n';
    snake->move();

    if (snake->equals(correct_snake)) {
        return PASSED;
    }

    return FAILED;

}

int main() {
    std::cout << "Snake::move() test 1: " << snake_move_test_1() << '\n';
    std::cout << "Snake::move() test 2: " << snake_move_test_2() << '\n';

    for (int i = 0; i < 10; i++) {
        std::cout << Apple::rand_float() << ' ';
    }
    std::cout << '\n';

    float f1 = 0.34f;
    float f2 = 0.34f;
    float f3 = 1.25f;
}


