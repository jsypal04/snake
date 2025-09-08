#include "game.hpp"
#include "cjson.h"
#include "env.hpp"
#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

extern struct game_state state;
pthread_mutex_t state_mutex;

// Helper function to send data
int send_state(std::string data, int sockfd) {
    const char* c_str_data = data.c_str();
    return write(sockfd, c_str_data, strlen(c_str_data));
}

void Game::connect_to_server() {

    if (pthread_create(&listener_thrd, NULL, listener, NULL) != 0) {
        perror("Failed to create thread");
        exit(1);
    }
    if (pthread_create(&sender_thrd, NULL, sender, NULL) != 0) {
        perror("Failed to create thread");
        exit(1);
    }
}

void* Game::listener(void *args) {
    std::cout << "Listener thread started...\n";

    while (true) {
        // listen for packet
        // deserialize packet
        // lock state
        // update state
        // unlock state
    }

    return NULL;
}

void* Game::sender(void* args) {
    std::cout << "Sender thread started...\n";

    int sockfd, n;
    struct sockaddr_in* serv_addr;
    struct addrinfo hints{}, *server;

    // create a socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cout << "ERROR: failed to open a socket.\n";
        return (void*)1;
    }

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int err = getaddrinfo(LOCAL_SERVER_ADDR, nullptr, &hints, &server);
    if (err != 0) {
        std::cout << "ERROR: failed to resolve hostname.\n";
        return (void*)1;
    }

    // copy server address info into serv_addr
    serv_addr = (struct sockaddr_in*)server->ai_addr;
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr*)serv_addr, sizeof(*serv_addr)) < 0) {
        std::cout << "ERROR: failed to open the socket.\n";
        return (void*)1;
    }

    char buff[256] = "hello world\n";
    n = write(sockfd, buff, strlen(buff));
    std:: cout << "n: " << n << '\n';

    while (true) {
        // lock state
        pthread_mutex_lock(&state_mutex);
        // serialize state
        std::string data = Game::serialize_state(state);
        std::cout << "State: " << data << '\n';

        // send state
        n = send_state(data, sockfd);
        // unlock state
        pthread_mutex_unlock(&state_mutex);
        // wait

        usleep(16);
    }

    return NULL;
}

std::string Game::serialize_state(struct game_state state) {
   Map* map_state = to_map(state);
   char* serialized_data = dump(map_state);
   destroyMap(map_state);
   return std::string(serialized_data);
}

struct game_state Game::deserialize_state(std::string json_data) {
    struct game_state state;
    int error;

    Map* map_state = load(json_data.c_str());
    state = from_map(map_state, &error);
    if (error != 0) {
        // return some error code
    }

    destroyMap(map_state);
    return state;
}


void Game::update_game_state(Snake* snake, Apple* apple) {
    int player_index = -1;
    struct snake player;
    std::vector<struct coord> coords;

    for (int i = 0; i < snake->length; i++) {
        coords.push_back({ .x = snake->squares[i]->x(), .y = snake->squares[i]->y()});
    }
    player.coords = coords;
    player.id = snake->id;

    pthread_mutex_lock(&state_mutex);

    state.apple_location = { .x = apple->x(), .y = apple->y() };
    for (int i = 0; i < state.num_players; i++) {
        if (state.players[i].id == snake->id) {
            player_index = i;
        }
    }

    if (player_index < 0) {
        state.players.push_back(player);
        state.num_players++;
    } else {
        state.players.erase(state.players.begin() + player_index);
        state.players.push_back(player);
    }

    pthread_mutex_unlock(&state_mutex);
}
