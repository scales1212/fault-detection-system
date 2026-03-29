#pragma once
#include "IDevice.hpp"
#include "UdpSocket.hpp"
#include <atomic>
#include <cstdint>
#include <memory>

/// @brief UDP receive-respond loop that drives one IDevice instance.
///
/// Binds a UDP socket to the given port, then loops:
///  1. Receive one datagram (blocking with timeout).
///  2. Parse as JSON and pass to device_->handle_command().
///  3. If the response is not null, send it back to the sender's address.
///
/// The null-response guard is what prevents inject commands from generating
/// a spurious UDP reply back to the GUI's ephemeral socket.
class DeviceSimulator {
public:
    /// @param device  Concrete device implementation (takes ownership).
    /// @param port    UDP port to bind (must match config/devices.json).
    DeviceSimulator(std::unique_ptr<IDevice> device, uint16_t port);

    /// @brief Start the receive-respond loop. Blocks until stop() is called.
    void run();

    /// @brief Signal run() to exit after the current recvfrom returns.
    void stop();

private:
    std::unique_ptr<IDevice> device_;
    UdpSocket                socket_;
    std::atomic<bool>        running_{false};
};
