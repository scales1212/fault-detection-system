#pragma once
#include "FaultIds.hpp"
#include <memory>
#include <string>
#include <spdlog/spdlog.h>

class FaultLogger {
public:
    // log_path: path to the output log file.
    explicit FaultLogger(const std::string& log_path);

    void log_send   (const std::string& uid, const std::string& payload);
    void log_receive(const std::string& uid, const std::string& payload);
    void log_timeout(const std::string& uid);

    void log_fault_transition(const std::string& uid,
                              const FaultId&     fid,
                              FaultState         state);

    void log_group_transition(const std::string& group_id,
                              FaultState         state);

    void log_cycle_overrun(long overrun_ms);

private:
    std::shared_ptr<spdlog::logger> logger_;
};
