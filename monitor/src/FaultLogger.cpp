#include "FaultLogger.hpp"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <stdexcept>
#include <string>
#include <vector>

FaultLogger::FaultLogger(const std::string& log_path) {
    auto file_sink    = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_path, true);
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    // Pattern: [timestamp] [level] message
    const std::string pattern = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v";
    file_sink->set_pattern(pattern);
    console_sink->set_pattern(pattern);

    logger_ = std::make_shared<spdlog::logger>(
        "fault_monitor",
        spdlog::sinks_init_list{file_sink, console_sink});

    logger_->set_level(spdlog::level::trace);
    logger_->flush_on(spdlog::level::info);
}

void FaultLogger::log_send(const std::string& uid, const std::string& payload) {
    logger_->info("[SEND]    uid={}  {}", uid, payload);
}

void FaultLogger::log_receive(const std::string& uid, const std::string& payload) {
    logger_->info("[RECV]    uid={}  {}", uid, payload);
}

void FaultLogger::log_timeout(const std::string& uid) {
    logger_->warn("[TIMEOUT] uid={}  no response received", uid);
}

void FaultLogger::log_fault_transition(const std::string& uid,
                                        const FaultId&     fid,
                                        FaultState         state) {
    // Build groups string: [g1,g2,...]
    std::string groups_str = "[";
    for (size_t i = 0; i < fid.groups.size(); ++i) {
        groups_str += fid.groups[i];
        if (i + 1 < fid.groups.size()) groups_str += ",";
    }
    groups_str += "]";

    const char* state_str = (state == FaultState::FAIL) ? "FAIL" : "PASS";

    if (state == FaultState::FAIL) {
        logger_->warn("[FAULT:FAIL]  uid={}  fault_id={}  numeric_id={}  groups={}",
                      uid, fid.id, fid.numeric_id, groups_str);
    } else {
        logger_->info("[FAULT:PASS]  uid={}  fault_id={}  numeric_id={}  groups={}",
                      uid, fid.id, fid.numeric_id, groups_str);
    }
    (void)state_str;
}

void FaultLogger::log_group_transition(const std::string& group_id, FaultState state) {
    if (state == FaultState::FAIL)
        logger_->warn("[GROUP:FAIL]  group={}", group_id);
    else
        logger_->info("[GROUP:PASS]  group={}", group_id);
}
