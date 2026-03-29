#include "DeviceProxy.hpp"
#include "FaultIds.hpp"
#include <stdexcept>

DeviceProxy::DeviceProxy(const std::string& uid, const std::string& device_type, uint16_t port)
    : uid_(uid), device_type_(device_type), socket_("127.0.0.1", port) {

    // Populate fault_fields_ from the generated fault ID registry (no hard-coded names)
    for (const auto& [key, fid] : getFaultIdRegistry()) {
        if (fid.device_type == device_type_)
            fault_fields_.push_back(fid.field);
    }
}

void DeviceProxy::send_command() {
    nlohmann::json cmd = {
        {"uid",     uid_},
        {"command", "poll"},
        {"value",   0}
    };
    socket_.send(cmd.dump());
}

bool DeviceProxy::try_receive() {
    auto raw = socket_.recv(5);
    if (!raw) return false;

    try {
        auto j = nlohmann::json::parse(*raw);
        response_queue_.push(std::move(j));
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<nlohmann::json> DeviceProxy::drain_queue() {
    std::vector<nlohmann::json> out;
    while (!response_queue_.empty()) {
        out.push_back(std::move(response_queue_.front()));
        response_queue_.pop();
    }
    return out;
}
