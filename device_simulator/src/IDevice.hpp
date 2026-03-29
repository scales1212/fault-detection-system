#pragma once
#include <string>
#include <nlohmann/json.hpp>

class IDevice {
public:
    virtual ~IDevice() = default;
    virtual nlohmann::json handle_command(const nlohmann::json& cmd) = 0;
    virtual std::string get_uid()  const = 0;
    virtual std::string get_type() const = 0;
};
