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

class Monitor {
public:
    Monitor(const std::string& config_path, const std::string& log_path);

    // Blocking 50Hz loop. Returns when stop() is called.
    void run();
    void stop();

private:
    ConfigLoader                          config_;
    FaultLogger                           logger_;
    FaultStateTracker                     fault_tracker_;
    std::vector<std::unique_ptr<DeviceProxy>> proxies_;
    std::atomic<bool>                     running_{false};

    static constexpr std::chrono::milliseconds k_cycle_period{20};  // 50 Hz
    static constexpr std::chrono::milliseconds k_recv_window{5};    // wait for responses

    void init_proxies();
    void execute_cycle();
    void check_faults(DeviceProxy& proxy, const nlohmann::json& resp);
};
