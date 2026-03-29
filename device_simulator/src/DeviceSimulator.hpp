#pragma once
#include "IDevice.hpp"
#include "UdpSocket.hpp"
#include <atomic>
#include <cstdint>
#include <memory>

class DeviceSimulator {
public:
    DeviceSimulator(std::unique_ptr<IDevice> device, uint16_t port);

    // Blocking receive-respond loop. Returns when stop() is called.
    void run();
    void stop();

private:
    std::unique_ptr<IDevice> device_;
    UdpSocket                socket_;
    std::atomic<bool>        running_{false};
};
