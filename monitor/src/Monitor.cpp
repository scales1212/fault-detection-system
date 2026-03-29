#include "Monitor.hpp"
#include <chrono>
#include <iostream>
#include <thread>

Monitor::Monitor(const std::string& config_path, const std::string& log_path)
    : config_(config_path), logger_(log_path),
      fault_tracker_(/* will init after config load */{}, getFaultIdRegistry()) {
    config_.load();
    // Re-construct tracker now that devices are loaded
    fault_tracker_ = FaultStateTracker(config_.get_devices(), getFaultIdRegistry());
    init_proxies();
}

void Monitor::init_proxies() {
    proxies_.clear();
    for (const auto& dev : config_.get_devices()) {
        proxies_.push_back(
            std::make_unique<DeviceProxy>(dev.uid, dev.type, dev.port));
    }
}

void Monitor::run() {
    using clock = std::chrono::steady_clock;
    running_ = true;
    std::cout << "Monitor running at 50Hz — " << proxies_.size() << " device(s)\n";

    while (running_) {
        auto cycle_start = clock::now();
        execute_cycle();

        auto deadline = cycle_start + k_cycle_period;
        auto now      = clock::now();
        if (now < deadline)
            std::this_thread::sleep_until(deadline);
        else
            std::cerr << "[WARN] Cycle overrun by "
                      << std::chrono::duration_cast<std::chrono::milliseconds>(now - deadline).count()
                      << "ms\n";
    }
}

void Monitor::stop() {
    running_ = false;
}

void Monitor::execute_cycle() {
    using clock = std::chrono::steady_clock;
    auto cycle_start = clock::now();

    // Phase 1 — send commands
    for (auto& proxy : proxies_) {
        proxy->send_command();
        logger_.log_send(proxy->get_uid(),
                         nlohmann::json{{"uid",proxy->get_uid()},
                                        {"command","poll"},{"value",0}}.dump());
    }

    // Phase 2 — brief wait for UDP responses to arrive
    std::this_thread::sleep_until(cycle_start + k_recv_window);

    // Phase 3 — receive and check faults
    for (auto& proxy : proxies_) {
        bool got = proxy->try_receive();
        if (got) {
            auto responses = proxy->drain_queue();
            for (const auto& resp : responses) {
                logger_.log_receive(proxy->get_uid(), resp.dump());
                check_faults(*proxy, resp);
            }
        } else {
            logger_.log_timeout(proxy->get_uid());
        }
    }
}

void Monitor::check_faults(DeviceProxy& proxy, const nlohmann::json& resp) {
    const auto& registry = getFaultIdRegistry();

    for (const auto& field : proxy.get_fault_fields()) {
        if (!resp.contains(field)) continue;

        bool current = resp[field].get<bool>();
        auto key     = std::make_pair(proxy.get_type(), field);
        auto it      = registry.find(key);
        if (it == registry.end()) continue;

        const FaultId& fid = it->second;
        auto result = fault_tracker_.update(proxy.get_uid(), proxy.get_type(), field, current);

        if (result.individual)
            logger_.log_fault_transition(proxy.get_uid(), fid, result.individual->second);

        for (const auto& [group_id, g_state] : result.groups)
            logger_.log_group_transition(group_id, g_state);
    }
}
