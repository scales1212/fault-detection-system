#include "DeviceSimulator.hpp"
#include <iostream>   // cerr for parse errors only
#include <nlohmann/json.hpp>

DeviceSimulator::DeviceSimulator(std::unique_ptr<IDevice> device, uint16_t port)
    : device_(std::move(device)), socket_(port) {}

void DeviceSimulator::run() {
    running_ = true;

    while (running_) {
        auto pkt = socket_.recv_from(200);
        if (!pkt) continue;

        nlohmann::json cmd;
        try {
            cmd = nlohmann::json::parse(pkt->data);
        } catch (...) {
            std::cerr << "Failed to parse command: " << pkt->data << "\n";
            continue;
        }

        nlohmann::json resp = device_->handle_command(cmd);
        if (!resp.is_null())
            socket_.send_to(resp.dump(), pkt->sender);
    }
}

void DeviceSimulator::stop() {
    running_ = false;
}
