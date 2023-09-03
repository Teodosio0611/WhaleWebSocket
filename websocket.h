#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <string>
#include <vector>

namespace Whale {
class WebSocketServer {
public:
    WebSocketServer() = default;
    ~WebSocketServer() = default;

    // Delete copy and move constructors and assign operators
    WebSocketServer(const WebSocketServer&) = delete;
    WebSocketServer& operator=(const WebSocketServer&) = delete;
    WebSocketServer(WebSocketServer&&) = delete;
    WebSocketServer& operator=(WebSocketServer&&) = delete;

    bool Start(std::string_view sockName);
    bool UpgradeWebSocket();
    void Stop();

    void Send(const std::string& message);

    void OnMessage(const std::string& message);
    void onError(const std::string& message);

private:
    WebSocketServer* server_;
    int fd_;
    int client_;
};
}

#endif // WEBSOCKET_SERVER_H