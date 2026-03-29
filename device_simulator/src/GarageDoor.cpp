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
    if (cmd.contains("inject")) {
        const auto& inj = cmd.at("inject");
        if (inj.contains("garage_open")  && inj["garage_open"].get<bool>())
            injected_garage_open_  = true;
        if (inj.contains("light_on")     && inj["light_on"].get<bool>())
            injected_light_on_     = true;
        if (inj.contains("over_voltage") && inj["over_voltage"].get<bool>())
            injected_over_voltage_ = true;
    }
    return build_response();
}
