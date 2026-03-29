#pragma once
#include <cstdint>
#include <optional>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

/// @brief Monitor-side UDP socket.
///
/// Binds an ephemeral local port on construction, sends datagrams to the fixed
/// remote (device simulator) address, and receives replies on that same socket.
///
/// Winsock2 is initialised in the constructor via WSAStartup; cleaned up in the
/// destructor via WSACleanup.
///
/// Not copyable — move semantics are intentionally omitted to keep it simple
/// (the monitor constructs sockets once at startup and never moves them).
class UdpSocket {
public:
    /// @param remote_host  IPv4 address or hostname of the device simulator.
    /// @param remote_port  UDP port the simulator is bound to.
    UdpSocket(const std::string& remote_host, uint16_t remote_port);
    ~UdpSocket();

    UdpSocket(const UdpSocket&)            = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    /// @brief Send data to the pre-configured remote address.
    /// @param data  Raw string payload (typically a JSON-serialised command).
    void send(const std::string& data);

    /// @brief Receive one datagram.
    /// @param timeout_ms  How long to wait before giving up (default 5 ms).
    /// @return Payload string, or nullopt on timeout or socket error.
    std::optional<std::string> recv(int timeout_ms = 5);

    /// Close the underlying socket early (also called by destructor).
    void close();

private:
    SOCKET      sock_   = INVALID_SOCKET;
    sockaddr_in remote_ = {};
    bool        closed_ = false;
};
