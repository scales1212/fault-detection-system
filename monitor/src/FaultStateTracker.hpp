#pragma once
#include "ConfigLoader.hpp"
#include "FaultIds.hpp"
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

/// @brief Holds the outcome of a single call to FaultStateTracker::update().
///
/// Either field may be empty: if nothing transitioned, both are empty.
struct TransitionResult {
    /// Set when the individual (uid, fault_id) pair changed state.
    /// Contains the FaultId descriptor and the new FaultState.
    std::optional<std::pair<FaultId, FaultState>> individual;

    /// One entry per group whose state changed as a result of the individual
    /// transition. Each entry is (group_id, new FaultState).
    std::vector<std::pair<std::string, FaultState>> groups;
};

/// @brief Tracks fault state at two levels:
///
///  1. **Individual** — one `(uid, fault_id)` pair per device instance per fault.
///  2. **Group** — one aggregate state per group_id, derived from all its members
///     across every device instance.
///
/// Only transitions are reported; repeated FAIL→FAIL or PASS→PASS calls return an
/// empty TransitionResult.
///
/// Group rules:
///  - → FAIL  : when an individual transitions to FAIL and the group is not already FAIL.
///  - → PASS  : when an individual transitions to PASS AND every member of the group
///               is now PASS or UNDEFINED (i.e. the last failing member cleared).
///
/// The tracker is initialised once with the full device list so it can pre-build the
/// group membership map before any cycle runs.
class FaultStateTracker {
public:
    using Registry = std::map<std::pair<std::string, std::string>, FaultId>;

    /// @param devices   All device instances from config/devices.json.
    /// @param registry  The global fault ID registry (from getFaultIdRegistry()).
    FaultStateTracker(const std::vector<DeviceConfig>& devices, const Registry& registry);

    /// @brief Evaluate a single field reading from a device response.
    ///
    /// @param uid           Device instance UID (e.g. "tcu-01").
    /// @param device_type   Device type string (e.g. "TemperatureControlUnit").
    /// @param field         Response field name (e.g. "overheating").
    /// @param current_value Current boolean value read from the response.
    /// @return Transitions that occurred. Both fields are empty if there was no change.
    TransitionResult update(const std::string& uid,
                            const std::string& device_type,
                            const std::string& field,
                            bool               current_value);

private:
    /// Individual fault state: key is (uid, fault_id_string).
    std::map<std::pair<std::string,std::string>, FaultState> individual_states_;

    /// Group aggregate state: key is group_id.
    std::map<std::string, FaultState> group_states_;

    /// Group membership: key is group_id, value is the set of (uid, fault_id) pairs
    /// that belong to it. Built at construction time from the device list × registry.
    std::map<std::string, std::set<std::pair<std::string,std::string>>> group_members_;

    /// @return true if every (uid, fault_id) member of group_id is PASS or UNDEFINED.
    bool all_group_members_non_fail(const std::string& group_id) const;
};
