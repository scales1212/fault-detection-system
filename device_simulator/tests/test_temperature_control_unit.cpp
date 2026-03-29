#include <gtest/gtest.h>
#include "TemperatureControlUnit.hpp"

// Normal poll — all faults false
TEST(TemperatureControlUnit, NormalResponseFaultsFalse) {
    TemperatureControlUnit tcu("tcu-test");
    nlohmann::json cmd = {{"uid","tcu-test"},{"command","poll"},{"value",0}};
    auto resp = tcu.handle_command(cmd);
    EXPECT_FALSE(resp["overheating"].get<bool>());
    EXPECT_FALSE(resp["out_of_bounds_temp"].get<bool>());
    EXPECT_EQ(resp["uid"].get<std::string>(), "tcu-test");
}

// Inject overheating — fires once then clears
TEST(TemperatureControlUnit, InjectOverheatingFiresOnce) {
    TemperatureControlUnit tcu("tcu-test");
    nlohmann::json inject_cmd = {
        {"uid","tcu-test"},{"command","poll"},{"value",0},
        {"inject",{{"overheating",true}}}
    };
    auto resp1 = tcu.handle_command(inject_cmd);
    EXPECT_TRUE(resp1["overheating"].get<bool>());

    // Second poll — latch should be cleared
    nlohmann::json normal_cmd = {{"uid","tcu-test"},{"command","poll"},{"value",0}};
    auto resp2 = tcu.handle_command(normal_cmd);
    EXPECT_FALSE(resp2["overheating"].get<bool>());
}

// Inject out_of_bounds_temp — fires once then clears
TEST(TemperatureControlUnit, InjectOutOfBoundsFiresOnce) {
    TemperatureControlUnit tcu("tcu-test");
    nlohmann::json inject_cmd = {
        {"uid","tcu-test"},{"command","poll"},{"value",0},
        {"inject",{{"out_of_bounds_temp",true}}}
    };
    auto resp1 = tcu.handle_command(inject_cmd);
    EXPECT_TRUE(resp1["out_of_bounds_temp"].get<bool>());

    nlohmann::json normal_cmd = {{"uid","tcu-test"},{"command","poll"},{"value",0}};
    auto resp2 = tcu.handle_command(normal_cmd);
    EXPECT_FALSE(resp2["out_of_bounds_temp"].get<bool>());
}

// Both fault fields can be injected independently
TEST(TemperatureControlUnit, BothFaultsIndependent) {
    TemperatureControlUnit tcu("tcu-test");

    nlohmann::json cmd1 = {{"uid","tcu-test"},{"command","poll"},{"value",0},
                           {"inject",{{"overheating",true}}}};
    auto r1 = tcu.handle_command(cmd1);
    EXPECT_TRUE(r1["overheating"].get<bool>());
    EXPECT_FALSE(r1["out_of_bounds_temp"].get<bool>());

    nlohmann::json cmd2 = {{"uid","tcu-test"},{"command","poll"},{"value",0},
                           {"inject",{{"out_of_bounds_temp",true}}}};
    auto r2 = tcu.handle_command(cmd2);
    EXPECT_FALSE(r2["overheating"].get<bool>());
    EXPECT_TRUE(r2["out_of_bounds_temp"].get<bool>());
}

// get_type returns expected string
TEST(TemperatureControlUnit, GetType) {
    TemperatureControlUnit tcu("tcu-test");
    EXPECT_EQ(tcu.get_type(), "TemperatureControlUnit");
    EXPECT_EQ(tcu.get_uid(), "tcu-test");
}
