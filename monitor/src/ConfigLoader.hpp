#pragma once
#include <cstdint>
#include <string>
#include <vector>

/// @brief Parsed representation of one entry in config/devices.json.
struct DeviceConfig {
    std::string uid;   ///< Unique instance identifier, e.g. "tcu-01".
    std::string type;  ///< Device type name, e.g. "TemperatureControlUnit".
    uint16_t    port;  ///< UDP port the device simulator listens on.
};

/// @brief Reads config/devices.json and exposes the device list.
///
/// Expected JSON format:
/// @code
/// {
///   "devices": [
///     { "uid": "tcu-01", "type": "TemperatureControlUnit", "port": 9001 },
///     { "uid": "gd-01",  "type": "GarageDoor",             "port": 9003 }
///   ]
/// }
/// @endcode
///
/// Throws std::runtime_error if the file is missing, unreadable, or malformed.
class ConfigLoader {
public:
    /// @param config_path  Path to the JSON configuration file.
    explicit ConfigLoader(const std::string& config_path);

    /// @brief Parse the configuration file. Must be called before get_devices().
    /// @throws std::runtime_error on file I/O errors or missing required fields.
    void load();

    /// @return The device list populated by the most recent call to load().
    const std::vector<DeviceConfig>& get_devices() const { return devices_; }

private:
    std::string               config_path_;
    std::vector<DeviceConfig> devices_;
};
