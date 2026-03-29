#include <gtest/gtest.h>
#include "FaultStateTracker.hpp"

// Helper: build a minimal device list with one TCU instance
static std::vector<DeviceConfig> one_tcu() {
    return {{"tcu-01", "TemperatureControlUnit", 9001}};
}

static std::vector<DeviceConfig> tcu_and_gd() {
    return {
        {"tcu-01", "TemperatureControlUnit", 9001},
        {"gd-01",  "GarageDoor",             9003}
    };
}

// UNDEFINED → FAIL on true
TEST(FaultStateTracker, UndefinedToFailOnTrue) {
    FaultStateTracker tracker(one_tcu(), getFaultIdRegistry());
    auto result = tracker.update("tcu-01", "TemperatureControlUnit", "overheating", true);
    ASSERT_TRUE(result.individual.has_value());
    EXPECT_EQ(result.individual->second, FaultState::FAIL);
    EXPECT_EQ(result.individual->first.id, "TCU_OVERHEAT");
}

// FAIL → PASS on false
TEST(FaultStateTracker, FailToPassOnFalse) {
    FaultStateTracker tracker(one_tcu(), getFaultIdRegistry());
    tracker.update("tcu-01", "TemperatureControlUnit", "overheating", true);
    auto result = tracker.update("tcu-01", "TemperatureControlUnit", "overheating", false);
    ASSERT_TRUE(result.individual.has_value());
    EXPECT_EQ(result.individual->second, FaultState::PASS);
}

// No transition when already FAIL and still true
TEST(FaultStateTracker, NoTransitionFailToFail) {
    FaultStateTracker tracker(one_tcu(), getFaultIdRegistry());
    tracker.update("tcu-01", "TemperatureControlUnit", "overheating", true);
    auto result = tracker.update("tcu-01", "TemperatureControlUnit", "overheating", true);
    EXPECT_FALSE(result.individual.has_value());
    EXPECT_TRUE(result.groups.empty());
}

// No transition when PASS and still false
TEST(FaultStateTracker, NoTransitionPassToPass) {
    FaultStateTracker tracker(one_tcu(), getFaultIdRegistry());
    tracker.update("tcu-01", "TemperatureControlUnit", "overheating", true);
    tracker.update("tcu-01", "TemperatureControlUnit", "overheating", false);
    auto result = tracker.update("tcu-01", "TemperatureControlUnit", "overheating", false);
    EXPECT_FALSE(result.individual.has_value());
    EXPECT_TRUE(result.groups.empty());
}

// Group transitions to FAIL when first member fails
TEST(FaultStateTracker, GroupFailsOnFirstMember) {
    FaultStateTracker tracker(one_tcu(), getFaultIdRegistry());
    auto result = tracker.update("tcu-01", "TemperatureControlUnit", "overheating", true);
    // overheating is in groups: temperature, safety
    ASSERT_FALSE(result.groups.empty());
    std::map<std::string, FaultState> gmap;
    for (const auto& [g, s] : result.groups) gmap[g] = s;
    EXPECT_EQ(gmap["temperature"], FaultState::FAIL);
    EXPECT_EQ(gmap["safety"],      FaultState::FAIL);
}

// Group doesn't get a second FAIL when already failed
TEST(FaultStateTracker, GroupNoDoubleFailEmission) {
    FaultStateTracker tracker(one_tcu(), getFaultIdRegistry());
    tracker.update("tcu-01", "TemperatureControlUnit", "overheating", true);
    // Now fail the second TCU fault (also in temperature group)
    auto result = tracker.update("tcu-01", "TemperatureControlUnit", "out_of_bounds_temp", true);
    // temperature group already FAIL — should NOT appear again
    for (const auto& [g, s] : result.groups)
        EXPECT_NE(g, "temperature");  // temperature already failed, no second event
}

// Group only passes when ALL members are clear (multi-fault scenario)
TEST(FaultStateTracker, GroupPassOnlyWhenAllClear) {
    FaultStateTracker tracker(one_tcu(), getFaultIdRegistry());
    // Fail both temperature faults
    tracker.update("tcu-01", "TemperatureControlUnit", "overheating",        true);
    tracker.update("tcu-01", "TemperatureControlUnit", "out_of_bounds_temp", true);
    // Clear overheating only — temperature group should still be FAIL
    auto r1 = tracker.update("tcu-01", "TemperatureControlUnit", "overheating", false);
    bool temp_passed = false;
    for (const auto& [g, s] : r1.groups)
        if (g == "temperature" && s == FaultState::PASS) temp_passed = true;
    EXPECT_FALSE(temp_passed);

    // Now clear the second fault — temperature group should PASS
    auto r2 = tracker.update("tcu-01", "TemperatureControlUnit", "out_of_bounds_temp", false);
    bool temp_passed2 = false;
    for (const auto& [g, s] : r2.groups)
        if (g == "temperature" && s == FaultState::PASS) temp_passed2 = true;
    EXPECT_TRUE(temp_passed2);
}

// Cross-device group: safety group stays FAIL if second device still failing
TEST(FaultStateTracker, CrossDeviceGroupHoldsUntilAllClear) {
    FaultStateTracker tracker(tcu_and_gd(), getFaultIdRegistry());
    // Fail safety from TCU side (overheating) and GD side (over_voltage)
    tracker.update("tcu-01", "TemperatureControlUnit", "overheating",  true);
    tracker.update("gd-01",  "GarageDoor",             "over_voltage", true);
    // Clear TCU fault — safety group still has GD_OVERVOLT in FAIL
    auto r = tracker.update("tcu-01", "TemperatureControlUnit", "overheating", false);
    bool safety_passed = false;
    for (const auto& [g, s] : r.groups)
        if (g == "safety" && s == FaultState::PASS) safety_passed = true;
    EXPECT_FALSE(safety_passed);

    // Clear GD fault — now safety should PASS
    auto r2 = tracker.update("gd-01", "GarageDoor", "over_voltage", false);
    bool safety_passed2 = false;
    for (const auto& [g, s] : r2.groups)
        if (g == "safety" && s == FaultState::PASS) safety_passed2 = true;
    EXPECT_TRUE(safety_passed2);
}

// Unknown field returns empty result (no crash)
TEST(FaultStateTracker, UnknownFieldIgnored) {
    FaultStateTracker tracker(one_tcu(), getFaultIdRegistry());
    auto result = tracker.update("tcu-01", "TemperatureControlUnit", "nonexistent_field", true);
    EXPECT_FALSE(result.individual.has_value());
    EXPECT_TRUE(result.groups.empty());
}
