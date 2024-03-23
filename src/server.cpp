#include "RedisServer.h"
#include "buttonrpc.hpp"

int main() {
   buttonrpc server;  
    server.as_server(5555);
    //server.bind("redis_command", redis_command);
    RedisServer::getInstance()->start();
    server.bind("redis_command", &RedisServer::handleClient, RedisServer::getInstance());
   // std::cout << "run rpc server on: " << 5555 << std::endl;
    server.run();


    RedisServer::getInstance()->start();
}
