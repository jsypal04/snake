#include "game.hpp"
#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

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
    const char* ENV = std::getenv("ENV");
    const char* SERV_ADDR;
    if (strcmp(ENV, "local") == 0) {
        SERV_ADDR = std::getenv("LOCAL_SERVER_ADDR");
    }
    else if (strcmp(ENV, "remote") == 0) {
        SERV_ADDR = std::getenv("REMOTE_SERVER_ADDR");
    }
    const char* PORT = std::getenv("PORT");

    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    
    portno = atoi(PORT);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error opening socket");
        return NULL;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = atoi(SERV_ADDR);
    serv_addr.sin_port = portno;
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error connecting to server");
        return NULL;
    }

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
    const char* ENV = std::getenv("ENV");
    const char* SERV_ADDR;
    if (strcmp(ENV, "local") == 0) {
        SERV_ADDR = std::getenv("LOCAL_SERVER_ADDR");
    }
    else if (strcmp(ENV, "remote") == 0) {
        SERV_ADDR = std::getenv("REMOTE_SERVER_ADDR");
    }
    const char* PORT = std::getenv("PORT");

    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    
    portno = atoi(PORT);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error opening socket");
        return NULL;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = atoi(SERV_ADDR);
    serv_addr.sin_port = portno;
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error connecting to server");
        return NULL;
    }

    while (true) {
        // lock state
        // serialize state
        // send state
        // unlock state
        // wait
    }

    return NULL;
}
