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

UdpSocket::UdpSocket(const std::string& remote_host, uint16_t remote_port) {
    init_winsock();

    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_ == INVALID_SOCKET)
        throw std::runtime_error("socket() failed: " + std::to_string(WSAGetLastError()));

    // Bind to any local port (ephemeral) so we can receive replies
    sockaddr_in local{};
    local.sin_family      = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port        = 0;
    if (bind(sock_, reinterpret_cast<sockaddr*>(&local), sizeof(local)) == SOCKET_ERROR)
        throw std::runtime_error("bind() failed: " + std::to_string(WSAGetLastError()));

    remote_.sin_family = AF_INET;
    remote_.sin_port   = htons(remote_port);
    if (inet_pton(AF_INET, remote_host.c_str(), &remote_.sin_addr) != 1)
        throw std::runtime_error("Invalid remote host: " + remote_host);
}

UdpSocket::~UdpSocket() { close(); }

void UdpSocket::send(const std::string& data) {
    if (closed_) return;
    sendto(sock_, data.c_str(), static_cast<int>(data.size()), 0,
           reinterpret_cast<const sockaddr*>(&remote_), sizeof(remote_));
}

std::optional<std::string> UdpSocket::recv(int timeout_ms) {
    if (closed_) return std::nullopt;

    DWORD tv = static_cast<DWORD>(timeout_ms);
    setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO,
               reinterpret_cast<const char*>(&tv), sizeof(tv));

    char buf[4096];
    int n = recvfrom(sock_, buf, sizeof(buf) - 1, 0, nullptr, nullptr);
    if (n == SOCKET_ERROR) return std::nullopt;

    buf[n] = '\0';
    return std::string(buf, n);
}

void UdpSocket::close() {
    if (!closed_ && sock_ != INVALID_SOCKET) {
        closesocket(sock_);
        sock_   = INVALID_SOCKET;
        closed_ = true;
    }
}
