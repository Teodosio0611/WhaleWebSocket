#include "websocket.h"

#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <sstream>
#include <openssl/sha.h>
#include <openssl/evp.h>

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

    return UpgradeWebSocket();
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

    if (lines.size() < 2) {
        return false;
    }
    if (lines[0].find("HTTP/1/1") != std::string_view::npos) {
        return false;
    }

    if (lines[2].find("Upgrade: websocket") == std::string_view::npos) {
        return false;
    }

    if (lines[4].find("Sec-WebSocket-Key: ") == std::string_view::npos) {
        return false;
    }

    if (lines[5].find("Sec-WebSocket-Version: 13") == std::string_view::npos) {
        return false;
    }

    std::string key = lines[4].substr(19).data();
    key.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    unsigned char hash[20];
    char acceptKey[32];
    SHA1(reinterpret_cast<const unsigned char*>(key.c_str()), strlen(reinterpret_cast<const char*>(key.c_str())), hash);
    EVP_EncodeBlock(reinterpret_cast<unsigned char*>(acceptKey), hash, 20);
    HttpReponse(acceptKey);
    return true;
}

bool WebSocketServer::HttpReponse(std::string_view message)
{
    std::ostringstream ss;
    ss << "HTTP/1.1 200 OK\r\n";
    ss << "Content-Type: text/plain\r\n";
    ss << "Content-Length: " << message.size() << "\r\n";
    ss << "Connection: close\r\n";
    ss << "\r\n";
    ss << message;
    std::string response = ss.str();
    write(client_, response.data(), response.size());
    return true;
}

void WebSocketServer::OnMessage()
{
    WsFrame frame;
    int index = 2;
    char buf[1024];
    read(client_, buf, sizeof(buf));
    frame.fin = (buf[0] >> 7) & 0x1;
    frame.opcode = buf[0] & 0xf;
    frame.mask = (buf[1] >> 7) & 0x1;
    frame.payload_len = buf[1] & 0x7f;
    if (frame.payload_len == 126) {
        index += 2;
        frame.payload_len = (buf[2] << 8) | buf[3];
    } else if (frame.payload_len == 127) {
        index += 8;
        frame.payload_len = (buf[2] << 56) | (buf[3] << 48) | (buf[4] << 40) | (buf[5] << 32) | (buf[6] << 24) | (buf[7] << 16) | (buf[8] << 8) | buf[9];
    }

    if (frame.mask == 1) {
        index += 4;
        frame.mask_key = (buf[index - 4] << 24) | (buf[index - 3] << 16) | (buf[index - 2] << 8) | buf[index - 1];
    }

    if (frame.mask == 1) {
        for (int i = 0; i < frame.payload_len; ++i) {
            buf[index + i] ^= buf[index + i % 4];
        }
    }

    frame.payload.reset(new char[frame.payload_len]);
    char* payload = frame.payload.get();
    memcpy(payload, buf + index, frame.payload_len);
    std::cout << frame.payload.get() << std::endl;
}
}