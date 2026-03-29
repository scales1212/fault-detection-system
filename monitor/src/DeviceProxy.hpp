#pragma once
#include "UdpSocket.hpp"
#include <nlohmann/json.hpp>
#include <optional>
#include <queue>
#include <string>
#include <vector>

class DeviceProxy {
public:
    DeviceProxy(const std::string& uid, const std::string& device_type, uint16_t port);

    // Send a poll command to the device.
    void send_command();

    // Non-blocking receive. Returns true if a response was queued.
    bool try_receive();

    // Drain all queued responses.
    std::vector<nlohmann::json> drain_queue();

    const std::string&              get_uid()          const { return uid_; }
    const std::string&              get_type()         const { return device_type_; }
    const std::vector<std::string>& get_fault_fields() const { return fault_fields_; }

private:
    std::string              uid_;
    std::string              device_type_;
    UdpSocket                socket_;
    std::queue<nlohmann::json> response_queue_;
    std::vector<std::string> fault_fields_;  // field names to check, from fault ID registry
};
