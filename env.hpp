#ifndef ENV_H
#define ENV_H

#include <string>

const std::string ENV = "local"; // or remote
const char* LOCAL_SERVER_ADDR = "localhost"; // decimal representation of 127.0.0.1 (localhost)
const std::string REMOTE_SERVER_ADDR = "singular-promptly-imp.ngrok-free.app";
const int PORT = 8000; // this is the server port
const int CLIENT_LISTENING_PORT = 8080;
const int LISTENER_BUFFER_SIZE = 1024;


#endif
