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
    for (int i = 0; i < snake.coords.size(); i++) {
        appendMap(&coords, coord_to_map(snake.coords.at(i)));
    }
    insertMapArray(&snake_map, (char*)"coords", coords);

    insertInt(&snake_map, (char*)"id", snake.id);
    return snake_map;
}

Map* to_map(struct game_state state) {
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

struct coord map_to_coord(Map* coord_map, int* error) {
    *error = 0;
    char type;
    void* value;

    struct coord coord;

    value = get(coord_map, (char*)"x", &type);
    if (value == NULL || type != FLOAT) {
        // return an error code
        *error = 1;
        return coord;
    }
    float x = *(float*)value;

    value = get(coord_map, (char*)"y", &type);
    if (value == NULL || type != FLOAT) {
        // return an error code
        *error = 1;
        return coord;
    }
    float y = *(float*)value;

    coord.x = x;
    coord.y = y;

    return coord;
}

struct snake map_to_snake(Map* snake_map, int* error) {
    *error = 0;
    char type;
    void* value;

    struct snake snake;

    value = get(snake_map, (char*)"coords", &type);
    if (value == NULL || type != ARRAY) {
        // return some error code
        *error = 1;
        return snake;
    }

    MapArray* coords_arr = (MapArray*)value;
    std::vector<struct coord> coords;

    for (int i = 0; i < coords_arr->size; i++) {
        Element coord = coords_arr->array[i];
        if (coord.type != MAP) {
            // return some error code
            *error = 1;
            return snake;
        }
        struct coord coord_struct = map_to_coord((Map*)coord.value, error);
        if (*error != 0) {
            return snake;
        }
        coords.push_back(coord_struct);
    }

    value = get(snake_map, (char*)"id", &type);
    if (value == NULL || type != INT) {
        // return an error code
        *error = 1;
        return snake;
    }
    int id = *(int*)value;

    snake.coords = coords;
    snake.id = id;

    return snake;
}

struct game_state from_map(Map* state_map, int* error) {
    *error = 0;
    char type;
    void* value;

    struct game_state state;

    value = get(state_map, (char*)"num_players", &type);
    if (value == NULL || type != INT) {
        // return some error code
        *error = 1;
        return state;
    }
    int num_players = *(int*)value;

    value = get(state_map, (char*)"apple_location", &type);
    if (value == NULL || type != MAP) {
        // return some error code
        *error = 1;
        return state;
    }
    struct coord apple_location = map_to_coord((Map*)value, error);
    if (error != 0) {
        return state;
    }

    value = get(state_map, (char*)"players", &type);
    if (value == NULL || type != ARRAY) {
        // return some error code
        *error = 1;
        return state;
    }
    MapArray* players_arr = (MapArray*)value;
    std::vector<struct snake> players;

    for (int i = 0; i < players_arr->size; i++) {
        Element player = players_arr->array[i];

        if (player.type != MAP) {
            // return some error code
            *error = 1;
            return state;
        }

        struct snake snake_struct = map_to_snake((Map*)player.value, error);
        if (error != 0) {
            return state;
        }
        players.push_back(snake_struct);
    }

    destroyMap(state_map);

    state.num_players = num_players;
    state.apple_location = apple_location;
    state.players = players;

    return state;
}
