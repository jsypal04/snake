#include "cjson.h"
#include "game.hpp"

const int num_players = 3;

enum result {
    PASSED,
    FAILED,
};

// HELPER FUNCTIONS:

Map* make_state_map() {
    Map* state_map = initMap(3);
    Map* apple_location = initMap(2);
    MapArray* players = initMapArray(num_players);

    insertFloat(&apple_location, (char*)"x", 0.0f);
    insertFloat(&apple_location, (char*)"y", 0.0f);

    for (int i = 0; i < num_players; i++) {
        Map* player = initMap(2);
        MapArray* coords = initMapArray(1);
        Map* c = initMap(2);

        insertFloat(&c, (char*)"x", 1.0f);
        insertFloat(&c, (char*)"y", 1.0f);

        appendMap(&coords, c);

        insertInt(&player, (char*)"id", 0);
        insertMapArray(&player, (char*)"coords", coords);

        appendMap(&players, player);
    }

    insertInt(&state_map, (char*)"num_players", num_players);
    insertMap(&state_map, (char*)"apple_location", apple_location);
    insertMapArray(&state_map, (char*)"players", players);

    return state_map;
}

struct game_state make_state() {

    struct game_state state;
    struct coord apple_location;
    apple_location.x = 0.0f;
    apple_location.y = 0.0f;
    std::vector<struct snake> players;
    for (int i = 0; i < num_players; i++) {
        std::vector<struct coord> coords;
        struct coord c;
        struct snake player;

        c.x = 1.0f;
        c.y = 1.0f;
        coords.push_back(c);
        player.coords = coords;
        player.id = 0;
        players.push_back(player);
    }

    state.apple_location = apple_location;
    state.num_players = num_players;
    state.players = players;

    return state;
}

// TESTS

enum result state_to_map_test() {
    std::cout << "Running state_to_map_test...\n";
    std::cout << "Created game state...\n";
    struct game_state state = make_state();
    std::cout << "Creating correct_map...\n";
    Map* correct_map = make_state_map();

    std::cout << "Converting game state to map...\n";
    Map* state_map = to_map(state);

    std::cout << "Comparing generated map and correct map...\n";
    if (map_cmp(state_map, correct_map)) {
        return PASSED;
    }

    return FAILED;
}

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
    std::cout << "state_to_map_test: " << state_to_map_test() << '\n';
}
