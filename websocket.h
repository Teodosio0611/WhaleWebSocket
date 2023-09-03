#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <string>
#include <vector>
#include <memory>

namespace Whale {
struct WsFrame
{
    int fin;
    int opcode;
    int mask;
    int mask_key;
    int payload_len;
    std::unique_ptr<char[]> payload;
};

enum class FrameType {
    CONTINUATION = 0x0,
    TEXT = 0x1,
    BINARY = 0x2,
    CLOSE = 0x8,
    PING = 0x9,
    PONG = 0xa
};

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
    bool HttpReponse(std::string_view message);
    void Stop();

    void Send(const std::string& message);

    void OnMessage();

private:
    WebSocketServer* server_;
    int fd_;
    int client_;
};
}

#endif // WEBSOCKET_SERVER_H