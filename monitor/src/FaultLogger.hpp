#pragma once
#include "FaultIds.hpp"
#include <memory>
#include <string>
#include <spdlog/spdlog.h>

/// @brief Structured logger for the monitor.
///
/// Uses two spdlog sinks:
///  - **File sink** (trace level) — receives every log entry; written immediately
///    (`flush_on(trace)`) so the GUI watchdog never misses a line.
///  - **Console sink** (err level) — silent during normal operation; only fires
///    for spdlog internals errors.
///
/// Log tag conventions:
///  - `[SEND]`       / `[RECV]`       — poll traffic (info)
///  - `[FAULT:FAIL]` / `[FAULT:PASS]` — individual fault transitions (warn / info)
///  - `[GROUP:FAIL]` / `[GROUP:PASS]` — group transitions (warn / info)
///  - `[TIMEOUT]`                      — device did not respond (warn)
///  - `[OVERRUN]`                      — 50 Hz cycle exceeded 20 ms (warn)
class FaultLogger {
public:
    /// @param log_path  Path to the output log file (created or appended).
    explicit FaultLogger(const std::string& log_path);

    /// Log a UDP poll command sent to a device.
    void log_send(const std::string& uid, const std::string& payload);

    /// Log a UDP response received from a device.
    void log_receive(const std::string& uid, const std::string& payload);

    /// Log that a device did not respond within the receive window.
    void log_timeout(const std::string& uid);

    /// @brief Log an individual fault state transition.
    ///
    /// Writes at warn level for FAIL, info level for PASS.
    ///
    /// @param uid    Device instance UID.
    /// @param fid    FaultId descriptor (id, numeric_id, groups).
    /// @param state  The new state (FAIL or PASS).
    void log_fault_transition(const std::string& uid,
                              const FaultId&     fid,
                              FaultState         state);

    /// @brief Log a group-level fault state transition.
    ///
    /// Writes at warn level for FAIL, info level for PASS.
    ///
    /// @param group_id  The group identifier (e.g. "temperature").
    /// @param state     The new state (FAIL or PASS).
    void log_group_transition(const std::string& group_id,
                              FaultState         state);

    /// Log that the 50 Hz cycle completed late.
    /// @param overrun_ms  How many milliseconds past the 20 ms deadline.
    void log_cycle_overrun(long overrun_ms);

private:
    std::shared_ptr<spdlog::logger> logger_;
};
