#include "game.hpp"
#include "cjson.h"
#include "env.hpp"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <pthread.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

pthread_mutex_t state_mutex;

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
        // serialize state
        // send state
        // unlock state
        // wait

        usleep(16);
    }

    return NULL;
}

struct game_state Game::deserialize_state(std::string json_data) {
    std::cout << "Running deserialize_state\n"; 
    /* const char* c_json_data = json_data.c_str();
    ObjectAST* ast_data = parse(c_json_data);
    Map* data = traverse_obj(ast_data);*/
}
