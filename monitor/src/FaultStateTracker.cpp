#include "FaultStateTracker.hpp"

FaultStateTracker::FaultStateTracker(const std::vector<DeviceConfig>& devices,
                                     const Registry& registry) {
    // Pre-build the group membership map and initialise all individual states.
    for (const auto& dev : devices) {
        for (const auto& [key, fid] : registry) {
            if (fid.device_type != dev.type) continue;

            auto ind_key = std::make_pair(dev.uid, fid.id);
            individual_states_[ind_key] = FaultState::UNDEFINED;

            for (const auto& g : fid.groups)
                group_members_[g].insert(ind_key);
        }
    }

    // Initialise all group states to UNDEFINED.
    for (const auto& [g, _] : group_members_)
        group_states_[g] = FaultState::UNDEFINED;
}

bool FaultStateTracker::all_group_members_non_fail(const std::string& group_id) const {
    auto it = group_members_.find(group_id);
    if (it == group_members_.end()) return true;
    for (const auto& member : it->second) {
        auto sit = individual_states_.find(member);
        if (sit != individual_states_.end() && sit->second == FaultState::FAIL)
            return false;
    }
    return true;
}

TransitionResult FaultStateTracker::update(const std::string& uid,
                                           const std::string& device_type,
                                           const std::string& field,
                                           bool               current_value) {
    TransitionResult result;

    // Look up the FaultId for this (device_type, field) pair
    const auto& registry = getFaultIdRegistry();
    auto reg_it = registry.find({device_type, field});
    if (reg_it == registry.end()) return result;  // not a tracked fault field

    const FaultId& fid = reg_it->second;
    auto ind_key = std::make_pair(uid, fid.id);

    auto& current_state = individual_states_[ind_key];
    FaultState new_individual = FaultState::UNDEFINED;
    bool transitioned = false;

    if (current_value && current_state != FaultState::FAIL) {
        new_individual = FaultState::FAIL;
        transitioned   = true;
    } else if (!current_value && current_state == FaultState::FAIL) {
        new_individual = FaultState::PASS;
        transitioned   = true;
    }

    if (!transitioned) return result;

    current_state = new_individual;
    result.individual = {fid, new_individual};

    // Evaluate group transitions
    for (const auto& group_id : fid.groups) {
        auto& g_state = group_states_[group_id];

        if (new_individual == FaultState::FAIL && g_state != FaultState::FAIL) {
            g_state = FaultState::FAIL;
            result.groups.emplace_back(group_id, FaultState::FAIL);
        } else if (new_individual == FaultState::PASS && g_state == FaultState::FAIL) {
            if (all_group_members_non_fail(group_id)) {
                g_state = FaultState::PASS;
                result.groups.emplace_back(group_id, FaultState::PASS);
            }
        }
    }

    return result;
}
