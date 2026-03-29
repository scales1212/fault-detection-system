#pragma once
#include <cstdint>
#include <optional>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

/// @brief Simulator-side UDP socket.
///
/// Unlike the monitor's UdpSocket, this one binds to a **known** port so that
/// clients (the monitor and the GUI) can address it by a fixed port number.
/// It uses `recvfrom` / `sendto` so that replies go back to whichever address
/// sent the command — the monitor and the GUI each use different source ports.
class UdpSocket {
public:
    /// @param bind_port  The well-known port this simulator listens on (from config).
    explicit UdpSocket(uint16_t bind_port);
    ~UdpSocket();

    UdpSocket(const UdpSocket&)            = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    /// @brief A received datagram plus the sender's address (needed to reply).
    struct Packet {
        std::string data;       ///< Raw payload string.
        sockaddr_in sender{};   ///< Source address captured by recvfrom.
    };

    /// @brief Block until a datagram arrives or the timeout elapses.
    /// @param timeout_ms  Maximum wait time in milliseconds (default 100 ms).
    /// @return Parsed packet, or nullopt on timeout or error.
    std::optional<Packet> recv_from(int timeout_ms = 100);

    /// @brief Send a reply back to the address from a previously received packet.
    /// @param data  Payload to send (typically a JSON-serialised response).
    /// @param addr  Destination address (use Packet::sender from recv_from).
    void send_to(const std::string& data, const sockaddr_in& addr);

    /// Close the socket early (also called by the destructor).
    void close();

private:
    SOCKET   sock_   = INVALID_SOCKET;
    uint16_t port_;
    bool     closed_ = false;
};
