#pragma once
#include <string>
#include <nlohmann/json.hpp>

/// @brief Abstract interface for all simulated device types.
///
/// Each concrete device (TemperatureControlUnit, GarageDoor, …) implements this
/// interface and is driven by DeviceSimulator, which dispatches every incoming
/// UDP command through handle_command().
///
/// Protocol contract:
///  - A "poll" command must return a JSON object containing the device's current
///    sensor state.
///  - An "inject" command sets an internal one-shot latch and returns a null JSON
///    value (`nlohmann::json()`). DeviceSimulator checks `is_null()` before
///    sending a UDP reply, so no response is sent for inject commands.
///  - Unknown commands should return a null or empty object.
class IDevice {
public:
    virtual ~IDevice() = default;

    /// @brief Process an incoming command and return the response.
    ///
    /// @param cmd  Parsed JSON command (must contain at least "command" field).
    /// @return     Sensor state JSON for "poll", null for "inject", null for unknown.
    virtual nlohmann::json handle_command(const nlohmann::json& cmd) = 0;

    /// @return The unique instance identifier set at construction (e.g. "tcu-01").
    virtual std::string get_uid()  const = 0;

    /// @return The device type string (e.g. "TemperatureControlUnit").
    virtual std::string get_type() const = 0;
};
