#pragma once
#include "IDevice.hpp"
#include <string>

class GarageDoor : public IDevice {
public:
    explicit GarageDoor(std::string uid);

    nlohmann::json handle_command(const nlohmann::json& cmd) override;
    std::string get_uid()  const override { return uid_; }
    std::string get_type() const override { return "GarageDoor"; }

private:
    std::string uid_;
    bool garage_open_  = false;
    bool light_on_     = false;
    bool over_voltage_ = false;

    // one-shot injection latches
    bool injected_garage_open_  = false;
    bool injected_light_on_     = false;
    bool injected_over_voltage_ = false;

    nlohmann::json build_response();
};
