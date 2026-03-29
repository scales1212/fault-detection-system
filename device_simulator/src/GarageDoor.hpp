#pragma once
#include "IDevice.hpp"
#include <string>

/// @brief Simulated garage door controller.
///
/// Tracks three boolean fault fields:
///  - `garage_open`  — the door is currently open.
///  - `light_on`     — the garage light is on when it should not be.
///  - `over_voltage` — the door motor supply voltage exceeds its rated limit.
///
/// Under normal operation all fields are false. Each field has an independent
/// one-shot inject latch: when set via an "inject" command, the field is true for
/// exactly one poll response, then resets to false.
///
/// This class has **no knowledge of fault IDs or groups**; it only manages raw
/// sensor state. The monitor is responsible for all fault detection logic.
class GarageDoor : public IDevice {
public:
    explicit GarageDoor(std::string uid);

    /// @brief Handle a "poll" or "inject" command.
    ///
    /// - "poll"   — applies any pending inject latches and returns current sensor
    ///              state as a JSON object.
    /// - "inject" — sets one-shot latches for the fields named in cmd["inject"]
    ///              and returns a null JSON value (no UDP response will be sent).
    nlohmann::json handle_command(const nlohmann::json& cmd) override;

    std::string get_uid()  const override { return uid_; }
    std::string get_type() const override { return "GarageDoor"; }

private:
    std::string uid_;

    bool garage_open_  = false;  ///< Door-open state.
    bool light_on_     = false;  ///< Light-on state.
    bool over_voltage_ = false;  ///< Over-voltage state.

    /// One-shot latches: set by "inject", consumed and cleared by the next "poll".
    bool injected_garage_open_  = false;
    bool injected_light_on_     = false;
    bool injected_over_voltage_ = false;

    /// Build and return the JSON response object reflecting current state.
    nlohmann::json build_response();
};
