#include "ConfigLoader.hpp"
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

ConfigLoader::ConfigLoader(const std::string& config_path)
    : config_path_(config_path) {}

void ConfigLoader::load() {
    std::ifstream f(config_path_);
    if (!f.is_open())
        throw std::runtime_error("Cannot open config: " + config_path_);

    nlohmann::json j;
    try {
        f >> j;
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("Config JSON parse error: " + std::string(e.what()));
    }

    if (!j.contains("devices") || !j["devices"].is_array())
        throw std::runtime_error("Config missing 'devices' array");

    devices_.clear();
    for (const auto& d : j["devices"]) {
        if (!d.contains("uid") || !d.contains("type") || !d.contains("port"))
            throw std::runtime_error("Device entry missing uid/type/port fields");

        DeviceConfig cfg;
        cfg.uid  = d["uid"].get<std::string>();
        cfg.type = d["type"].get<std::string>();
        cfg.port = static_cast<uint16_t>(d["port"].get<int>());
        devices_.push_back(std::move(cfg));
    }
}
