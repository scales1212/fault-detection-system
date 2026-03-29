#include "TemperatureControlUnit.hpp"

TemperatureControlUnit::TemperatureControlUnit(std::string uid)
    : uid_(std::move(uid)) {}

void TemperatureControlUnit::simulate_step() {
    // Slowly drift current_temp toward set_temp with small noise
    current_temp_ += (set_temp_ - current_temp_) * 0.05f;
    overheating_        = current_temp_ > 35.0f;
    out_of_bounds_temp_ = current_temp_ < 5.0f || current_temp_ > 40.0f;
}

nlohmann::json TemperatureControlUnit::build_response() {
    bool oh  = overheating_        || injected_overheating_;
    bool oob = out_of_bounds_temp_ || injected_out_of_bounds_temp_;
    injected_overheating_        = false;
    injected_out_of_bounds_temp_ = false;
    return nlohmann::json{
        {"uid",               uid_},
        {"command",           "response"},
        {"current_temp",      current_temp_},
        {"set_temp",          set_temp_},
        {"overheating",       oh},
        {"out_of_bounds_temp", oob}
    };
}

nlohmann::json TemperatureControlUnit::handle_command(const nlohmann::json& cmd) {
    simulate_step();

    // Apply injection latches from GUI
    if (cmd.contains("inject")) {
        const auto& inj = cmd.at("inject");
        if (inj.contains("overheating") && inj["overheating"].get<bool>())
            injected_overheating_ = true;
        if (inj.contains("out_of_bounds_temp") && inj["out_of_bounds_temp"].get<bool>())
            injected_out_of_bounds_temp_ = true;
    }

    // Accept set_temp updates
    if (cmd.contains("value") && cmd.contains("command") &&
        cmd["command"].get<std::string>() == "set_temp") {
        set_temp_ = cmd["value"].get<float>();
    }

    return build_response();
}
