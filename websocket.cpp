#include "websocket.h"
#include "utils.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>

namespace Whale {
bool WebSocketServer::Start(std::string_view sockName)
{
    fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd_ < 0) {
        return false;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, sockName.data());
    addr.sun_path[0] = 0;
    int32_t len = offsetof(sockaddr_un, sun_path) + strlen(addr.sun_path) + 1;
    if (bind(fd_, (sockaddr*)&addr, len) < 0) {
        return false;
    }

    if (listen(fd_, 5) < 0) {
        return false;
    }
    
    client_ = accept(fd_, nullptr, nullptr);
    if (client_ < 0) {
        return false;
    }

    UpgradeWebSocket();
}

// split string_view by delimiter
std::vector<std::string_view> Split(std::string_view str, std::string_view delimiter)
{
    std::vector<std::string_view> result;
    size_t pos = 0;
    while (pos < str.size()) {
        size_t eol = str.find(delimiter, pos);
        if (eol == std::string_view::npos) {
            break;
        }
        result.push_back(str.substr(pos, eol - pos));
        pos = eol + delimiter.size();
    }
    return result;
}

std::tuple<std::string_view, std::string_view, std::string_view> ParseRequestLine(std::string_view line)
{
    auto parts = Split(line, " ");
    if (parts.size() != 3) {
        return std::make_tuple("", "", "");
    }
    return std::make_tuple(parts[0], parts[1], parts[2]);
}

std::tuple<std::string_view, std::string_view> ParseHeader(std::string_view line)
{
    auto parts = Split(line, ": ");
    if (parts.size() != 2) {
        return std::make_tuple("", "");
    }
    return std::make_tuple(parts[0], parts[1]);
}


bool WebSocketServer::UpgradeWebSocket()
{
    char buf[1024];
    int32_t n = read(client_, buf, sizeof(buf));
    if (n < 0) {
        return false;
    }

    std::string_view request(buf, n);
    std::vector<std::string_view> lines;
    size_t pos = 0;
    while (pos < request.size()) {
        size_t eol = request.find("\r\n", pos);
        if (eol == std::string_view::npos) {
            break;
        }
        lines.push_back(request.substr(pos, eol - pos));
        pos = eol + 2;
    }

    if (lines.size() < 3) {
        return false;
    }

    std::string_view method, path, version;
    auto [method, path, version] = ParseRequestLine(lines[0]);
    if (method != "GET") {
        return false;
    }

    std::string_view connection, upgrade, secWebSocketKey;
    auto [connection, upgrade] = ParseHeader(lines[1]);
    if (connection != "Upgrade" || upgrade != "websocket") {
        return false;
    }

    std::string_view secWebSocketVersion;
    auto [secWebSocketVersion, secWebSocketKey] = ParseHeader(lines[2]);
    if (secWebSocketVersion != "Sec-WebSocket-Version" || secWebSocketKey != "13") {
        return false;
    }

    std::string_view secWebSocketAccept;
    auto [secWebSocketAccept, secWebSocketKey] = ParseHeader(lines[3]);
    if (secWebSocketAccept != "Sec-WebSocket-Accept") {
        return false;
    }

    std::string_view secWebSocketProtocol;
    std::tie(secWebSocketProtocol, secWebSocketKey) = ParseHeader(lines[4]);
    if (secWebSocketProtocol != "Sec-WebSocket-Protocol") {
        return false;
    }

    std::string_view origin;
    std::tie(origin, secWebSocketKey) = ParseHeader(lines[5]);
    if (origin != "Origin") {
        return false;
    }

    std::string_view secWebSocketExtensions;
    std::tie(secWebSocketExtensions, secWebSocketKey) = ParseHeader(lines[6]);
    if (secWebSocketExtensions != "Sec-WebSocket-Extensions") {
        return false;
    }

    std::string_view emptyLine;
    std::
}
}