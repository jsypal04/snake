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
    
    MapArray* coords = initMapArray(sizeof(snake.coords));
    for (int i = 0; i < sizeof(snake.coords); i++) {
        appendMap(&coords, coord_to_map(snake.coords.at(i)));
    }
    insertMapArray(&snake_map, (char*)"coords", coords);

    insertInt(&snake_map, (char*)"id", snake.id);
    return snake_map;
}

Map* state_to_map(struct game_state state) {
    Map* map_state = initMap(3);

    insertMap(&map_state, (char*)"apple_location", coord_to_map(state.apple_location));
    
    MapArray* players = initMapArray(state.num_players);
    for (int i = 0; i < state.num_players; i++) {
        appendMap(&players, snake_to_map(state.players.at(i)));
    }
    insertMapArray(&map_state, (char*)"players", players);

    insertInt(&map_state, (char*)"num_players", state.num_players);

    return map_state;
}
