#include "cjson.h"
#include "game.hpp"

// TODO: implement
Map* coord_to_map(struct coord coord);

// TODO: implement
Map* snake_to_map(struct snake snake);

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
