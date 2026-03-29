#pragma once
#include "ConfigLoader.hpp"
#include "DeviceProxy.hpp"
#include "FaultLogger.hpp"
#include "FaultStateTracker.hpp"
#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

/// @brief Top-level monitor: runs a 50 Hz polling loop over all configured
///        device proxies, evaluates fault state transitions, and logs results.
///
/// Construction order matters: ConfigLoader must load before FaultStateTracker
/// is built, because the tracker needs the full device list to pre-populate its
/// group membership map.
class Monitor {
public:
    /// @param config_path  Path to config/devices.json.
    /// @param log_path     Path where fault_monitor.log is written.
    Monitor(const std::string& config_path, const std::string& log_path);

    /// @brief Start the 50 Hz loop. Blocks until stop() is called.
    void run();

    /// @brief Signal the run() loop to exit after the current cycle completes.
    void stop();

private:
    ConfigLoader                              config_;
    FaultLogger                               logger_;
    FaultStateTracker                         fault_tracker_;
    std::vector<std::unique_ptr<DeviceProxy>> proxies_;
    std::atomic<bool>                         running_{false};

    /// Fixed 20 ms period → 50 Hz.
    static constexpr std::chrono::milliseconds k_cycle_period{20};
    /// Window after sending commands during which responses are expected.
    static constexpr std::chrono::milliseconds k_recv_window{5};

    /// Create one DeviceProxy per device entry from the loaded config.
    void init_proxies();

    /// Execute one full poll-receive-check cycle.
    void execute_cycle();

    /// @brief For each fault field registered for proxy's device type, read the
    ///        field from resp, call FaultStateTracker::update(), and log any
    ///        state transitions that result.
    void check_faults(DeviceProxy& proxy, const nlohmann::json& resp);
};
