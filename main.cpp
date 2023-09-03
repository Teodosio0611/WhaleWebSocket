#include <iostream>
#include "websocket.h"
int main() {
    using namespace Whale;
    WebSocketServer* server = new WebSocketServer();
    std::string sock = "PandaDebugger";
    server->Start(sock);
}