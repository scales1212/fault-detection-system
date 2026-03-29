#pragma once
#include <cstdint>
#include <optional>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

// Monitor-side UDP socket: binds an ephemeral local port, sends to a fixed
// remote address, and receives on that same local socket.
class UdpSocket {
public:
    UdpSocket(const std::string& remote_host, uint16_t remote_port);
    ~UdpSocket();

    UdpSocket(const UdpSocket&)            = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    void send(const std::string& data);

    // Returns received payload, or nullopt on timeout/error.
    std::optional<std::string> recv(int timeout_ms = 5);

    void close();

private:
    SOCKET     sock_    = INVALID_SOCKET;
    sockaddr_in remote_ = {};
    bool        closed_ = false;
};
