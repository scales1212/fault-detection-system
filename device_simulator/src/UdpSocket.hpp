#pragma once
#include <cstdint>
#include <optional>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

// Simulator-side UDP socket: binds to a known port, uses recvfrom/sendto
// so it can reply back to whoever sent the command.
class UdpSocket {
public:
    explicit UdpSocket(uint16_t bind_port);
    ~UdpSocket();

    UdpSocket(const UdpSocket&)            = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    struct Packet {
        std::string    data;
        sockaddr_in    sender{};
    };

    // Block until a packet arrives or timeout_ms elapses. Returns nullopt on timeout.
    std::optional<Packet> recv_from(int timeout_ms = 100);

    // Send data back to the address captured by recv_from.
    void send_to(const std::string& data, const sockaddr_in& addr);

    void close();

private:
    SOCKET      sock_   = INVALID_SOCKET;
    uint16_t    port_;
    bool        closed_ = false;
};
