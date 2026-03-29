#pragma once
#include "IDevice.hpp"
#include <string>

class TemperatureControlUnit : public IDevice {
public:
    explicit TemperatureControlUnit(std::string uid);

    nlohmann::json handle_command(const nlohmann::json& cmd) override;
    std::string get_uid()  const override { return uid_; }
    std::string get_type() const override { return "TemperatureControlUnit"; }

private:
    std::string uid_;
    float current_temp_       = 20.0f;
    float set_temp_           = 22.0f;
    bool  overheating_        = false;
    bool  out_of_bounds_temp_ = false;

    // one-shot injection latches (set by GUI, cleared after one response)
    bool injected_overheating_        = false;
    bool injected_out_of_bounds_temp_ = false;

    void simulate_step();
    nlohmann::json build_response();
};
