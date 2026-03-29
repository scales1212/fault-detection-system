#pragma once
#include "IDevice.hpp"
#include <string>

/// @brief Simulated temperature control unit.
///
/// Tracks two boolean fault fields:
///  - `overheating`        — the unit's temperature has exceeded a safe threshold.
///  - `out_of_bounds_temp` — the measured temperature is outside the valid sensor range.
///
/// Under normal operation both fields are false. A fault injection latch for each field
/// can be set via an "inject" command. The latch fires exactly once — it sets the
/// corresponding field to true in the next poll response, then clears automatically.
///
/// This class has **no knowledge of fault IDs or groups**; it only manages raw sensor
/// state. The monitor is responsible for all fault detection logic.
class TemperatureControlUnit : public IDevice {
public:
    explicit TemperatureControlUnit(std::string uid);

    /// @brief Handle a "poll" or "inject" command.
    ///
    /// - "poll"   — advances the simulation one step and returns the current
    ///              sensor state as a JSON object. Inject latches fire here.
    /// - "inject" — sets one-shot latches for the fields named in cmd["inject"]
    ///              and returns a null JSON value (no UDP response will be sent).
    nlohmann::json handle_command(const nlohmann::json& cmd) override;

    std::string get_uid()  const override { return uid_; }
    std::string get_type() const override { return "TemperatureControlUnit"; }

private:
    std::string uid_;

    float current_temp_       = 20.0f;  ///< Simulated current temperature (°C).
    float set_temp_           = 22.0f;  ///< Target set-point temperature (°C).
    bool  overheating_        = false;  ///< Active overheating state.
    bool  out_of_bounds_temp_ = false;  ///< Active out-of-bounds temperature state.

    /// One-shot latch: set by "inject", consumed and cleared by the next "poll".
    bool injected_overheating_        = false;
    bool injected_out_of_bounds_temp_ = false;

    /// Advance internal simulation state and apply any pending inject latches.
    void simulate_step();

    /// Build and return the JSON response object reflecting current state.
    nlohmann::json build_response();
};
