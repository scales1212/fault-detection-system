#pragma once
#include "ConfigLoader.hpp"
#include "FaultIds.hpp"
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

struct TransitionResult {
    // Set if the individual fault transitioned; contains (FaultId, new state).
    std::optional<std::pair<FaultId, FaultState>> individual;
    // Group transitions that occurred as a result: (group_id, new_state).
    std::vector<std::pair<std::string, FaultState>> groups;
};

class FaultStateTracker {
public:
    using Registry = std::map<std::pair<std::string, std::string>, FaultId>;

    FaultStateTracker(const std::vector<DeviceConfig>& devices, const Registry& registry);

    // Update state for one field of one device instance.
    // Returns transitions that occurred (may be empty if no state change).
    TransitionResult update(const std::string& uid,
                            const std::string& device_type,
                            const std::string& field,
                            bool               current_value);

private:
    // Key: (uid, fault_id_string) → FaultState
    std::map<std::pair<std::string,std::string>, FaultState> individual_states_;

    // Key: group_id → FaultState
    std::map<std::string, FaultState> group_states_;

    // Key: group_id → set of (uid, fault_id_string) members
    std::map<std::string, std::set<std::pair<std::string,std::string>>> group_members_;

    bool all_group_members_non_fail(const std::string& group_id) const;
};
