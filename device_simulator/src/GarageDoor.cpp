#include "GarageDoor.hpp"

GarageDoor::GarageDoor(std::string uid)
    : uid_(std::move(uid)) {}

nlohmann::json GarageDoor::build_response() {
    bool go  = garage_open_  || injected_garage_open_;
    bool lo  = light_on_     || injected_light_on_;
    bool ov  = over_voltage_ || injected_over_voltage_;
    injected_garage_open_  = false;
    injected_light_on_     = false;
    injected_over_voltage_ = false;
    return nlohmann::json{
        {"uid",          uid_},
        {"command",      "response"},
        {"garage_open",  go},
        {"light_on",     lo},
        {"over_voltage", ov}
    };
}

nlohmann::json GarageDoor::handle_command(const nlohmann::json& cmd) {
    const std::string command = cmd.value("command", "");

    // Inject-only command from GUI: set latches, return null (no UDP response sent).
    if (command == "inject") {
        if (cmd.contains("inject")) {
            const auto& inj = cmd.at("inject");
            if (inj.value("garage_open",  false)) injected_garage_open_  = true;
            if (inj.value("light_on",     false)) injected_light_on_     = true;
            if (inj.value("over_voltage", false)) injected_over_voltage_ = true;
        }
        return nlohmann::json();  // null — DeviceSimulator must not send a response
    }

    return build_response();
}
