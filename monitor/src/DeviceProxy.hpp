#pragma once
#include "UdpSocket.hpp"
#include <nlohmann/json.hpp>
#include <optional>
#include <queue>
#include <string>
#include <vector>

/// @brief Represents one device instance from the monitor's perspective.
///
/// Each proxy owns a UDP socket configured for its device's host:port, and
/// maintains a list of fault fields that should be checked in every response.
/// The fault field list is derived from the generated FaultIds registry filtered
/// by device type — no field names are hard-coded here.
///
/// Typical cycle usage:
/// @code
///     proxy.send_command();            // send poll over UDP
///     // ... sleep for receive window ...
///     if (proxy.try_receive()) {
///         for (auto& resp : proxy.drain_queue())
///             check_faults(proxy, resp);
///     }
/// @endcode
class DeviceProxy {
public:
    /// @param uid          Device instance UID (e.g. "tcu-01").
    /// @param device_type  Device type string (e.g. "TemperatureControlUnit").
    /// @param port         UDP port the simulator is listening on.
    DeviceProxy(const std::string& uid, const std::string& device_type, uint16_t port);

    /// Send a poll command to the device simulator over UDP.
    void send_command();

    /// @brief Attempt to receive one UDP response (non-blocking with 5 ms timeout).
    /// @return true if at least one response was queued; false on timeout.
    bool try_receive();

    /// @brief Remove and return all queued responses.
    /// @return Parsed JSON objects, one per response received since last drain.
    std::vector<nlohmann::json> drain_queue();

    const std::string&              get_uid()          const { return uid_; }
    const std::string&              get_type()         const { return device_type_; }

    /// @return Field names (e.g. "overheating") that the monitor checks for faults.
    ///         Populated at construction from the fault ID registry, not hard-coded.
    const std::vector<std::string>& get_fault_fields() const { return fault_fields_; }

private:
    std::string               uid_;
    std::string               device_type_;
    UdpSocket                 socket_;
    std::queue<nlohmann::json> response_queue_;

    /// Field names to check in every response, derived from getFaultIdRegistry()
    /// filtered by device_type_.
    std::vector<std::string>  fault_fields_;
};
