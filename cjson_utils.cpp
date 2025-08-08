#include "cjson.h"
#include "game.hpp"

Map* coord_to_map(struct coord coord) {
    Map* coord_map = initMap(2);
    insertFloat(&coord_map, (char*)"x", coord.x);
    insertFloat(&coord_map, (char*)"y", coord.y);
    return coord_map;
}

Map* snake_to_map(struct snake snake) {
    Map* snake_map = initMap(2);

    MapArray* coords = initMapArray(snake.coords.size());
    for (int i = 0; i < sizeof(snake.coords); i++) {
        appendMap(&coords, coord_to_map(snake.coords.at(i)));
    }
    insertMapArray(&snake_map, (char*)"coords", coords);

    insertInt(&snake_map, (char*)"id", snake.id);
    return snake_map;
}

Map* to_map(struct game_state state) {
    std::cout << "Running to_map...\n";
    std::cout << "Initializing map...\n";
    Map* map_state = initMap(3);

    std::cout << "Inserting apple_location...\n";
    insertMap(&map_state, (char*)"apple_location", coord_to_map(state.apple_location));

    std::cout << "Initializing map array...\n";
    MapArray* players = initMapArray(state.num_players);
    for (int i = 0; i < state.num_players; i++) {
        std::cout << "Adding player " << i << " to the players map array...\n";
        appendMap(&players, snake_to_map(state.players.at(i)));
    }
    std::cout << "Inserting players into map...\n";
    insertMapArray(&map_state, (char*)"players", players);

    std::cout << "Inserting num_players into the map...\n";
    insertInt(&map_state, (char*)"num_players", state.num_players);

    std:: cout << "Returning the map...\n";
    return map_state;
}
