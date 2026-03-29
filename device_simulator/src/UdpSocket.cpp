#include "UdpSocket.hpp"
#include <stdexcept>
#include <string>
#pragma comment(lib, "Ws2_32.lib")

namespace {
void init_winsock() {
    static bool done = false;
    if (!done) {
        WSADATA wd{};
        if (WSAStartup(MAKEWORD(2, 2), &wd) != 0)
            throw std::runtime_error("WSAStartup failed");
        done = true;
    }
}
} // namespace

UdpSocket::UdpSocket(uint16_t bind_port) : port_(bind_port) {
    init_winsock();

    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_ == INVALID_SOCKET)
        throw std::runtime_error("socket() failed: " + std::to_string(WSAGetLastError()));

    // Allow address reuse so restarts don't block on TIME_WAIT
    int opt = 1;
    setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port        = htons(bind_port);

    if (bind(sock_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
        throw std::runtime_error("bind() failed on port " + std::to_string(bind_port) +
                                 ": " + std::to_string(WSAGetLastError()));
}

UdpSocket::~UdpSocket() { close(); }

std::optional<UdpSocket::Packet> UdpSocket::recv_from(int timeout_ms) {
    if (closed_) return std::nullopt;

    DWORD tv = static_cast<DWORD>(timeout_ms);
    setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO,
               reinterpret_cast<const char*>(&tv), sizeof(tv));

    char buf[4096];
    sockaddr_in sender{};
    int sender_len = sizeof(sender);

    int n = recvfrom(sock_, buf, sizeof(buf) - 1, 0,
                     reinterpret_cast<sockaddr*>(&sender), &sender_len);
    if (n == SOCKET_ERROR) return std::nullopt;

    buf[n] = '\0';
    return Packet{std::string(buf, n), sender};
}

void UdpSocket::send_to(const std::string& data, const sockaddr_in& addr) {
    if (closed_) return;
    sendto(sock_, data.c_str(), static_cast<int>(data.size()), 0,
           reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
}

void UdpSocket::close() {
    if (!closed_ && sock_ != INVALID_SOCKET) {
        closesocket(sock_);
        sock_   = INVALID_SOCKET;
        closed_ = true;
    }
}
