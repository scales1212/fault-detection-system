#include "TemperatureControlUnit.hpp"

TemperatureControlUnit::TemperatureControlUnit(std::string uid)
    : uid_(std::move(uid)) {}

void TemperatureControlUnit::simulate_step() {
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
        {"uid",                uid_},
        {"command",            "response"},
        {"current_temp",       current_temp_},
        {"set_temp",           set_temp_},
        {"overheating",        oh},
        {"out_of_bounds_temp", oob}
    };
}

nlohmann::json TemperatureControlUnit::handle_command(const nlohmann::json& cmd) {
    const std::string command = cmd.value("command", "");

    // Inject-only command from GUI: set latches, return null (no UDP response sent).
    // Latch fires on the NEXT monitor poll so the monitor actually sees the fault.
    if (command == "inject") {
        if (cmd.contains("inject")) {
            const auto& inj = cmd.at("inject");
            if (inj.value("overheating",        false)) injected_overheating_        = true;
            if (inj.value("out_of_bounds_temp", false)) injected_out_of_bounds_temp_ = true;
        }
        return nlohmann::json();  // null — DeviceSimulator must not send a response
    }

    simulate_step();

    if (command == "set_temp")
        set_temp_ = cmd.value("value", set_temp_);

    return build_response();
}
