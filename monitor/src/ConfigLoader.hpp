#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct DeviceConfig {
    std::string uid;
    std::string type;
    uint16_t    port;
};

class ConfigLoader {
public:
    explicit ConfigLoader(const std::string& config_path);

    void load();
    const std::vector<DeviceConfig>& get_devices() const { return devices_; }

private:
    std::string              config_path_;
    std::vector<DeviceConfig> devices_;
};
