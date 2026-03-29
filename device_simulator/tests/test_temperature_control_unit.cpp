#include <gtest/gtest.h>
#include "TemperatureControlUnit.hpp"

static const nlohmann::json kPoll = {{"uid","tcu-test"},{"command","poll"},{"value",0}};

// Normal poll — all faults false
TEST(TemperatureControlUnit, NormalResponseFaultsFalse) {
    TemperatureControlUnit tcu("tcu-test");
    auto resp = tcu.handle_command(kPoll);
    EXPECT_FALSE(resp["overheating"].get<bool>());
    EXPECT_FALSE(resp["out_of_bounds_temp"].get<bool>());
    EXPECT_EQ(resp["uid"].get<std::string>(), "tcu-test");
}

// Inject command returns null (no UDP response sent to GUI)
TEST(TemperatureControlUnit, InjectCommandReturnsNull) {
    TemperatureControlUnit tcu("tcu-test");
    nlohmann::json inject = {{"uid","tcu-test"},{"command","inject"},{"inject",{{"overheating",true}}}};
    auto r = tcu.handle_command(inject);
    EXPECT_TRUE(r.is_null());
}

// Inject sets latch; latch fires on next poll then clears
TEST(TemperatureControlUnit, InjectOverheatingFiresOnNextPoll) {
    TemperatureControlUnit tcu("tcu-test");
    nlohmann::json inject = {{"uid","tcu-test"},{"command","inject"},{"inject",{{"overheating",true}}}};
    tcu.handle_command(inject);  // sets latch, returns null

    auto resp1 = tcu.handle_command(kPoll);   // latch fires here
    EXPECT_TRUE(resp1["overheating"].get<bool>());

    auto resp2 = tcu.handle_command(kPoll);   // latch cleared
    EXPECT_FALSE(resp2["overheating"].get<bool>());
}

// out_of_bounds_temp latch fires on next poll then clears
TEST(TemperatureControlUnit, InjectOutOfBoundsFiresOnNextPoll) {
    TemperatureControlUnit tcu("tcu-test");
    nlohmann::json inject = {{"uid","tcu-test"},{"command","inject"},{"inject",{{"out_of_bounds_temp",true}}}};
    tcu.handle_command(inject);

    auto resp1 = tcu.handle_command(kPoll);
    EXPECT_TRUE(resp1["out_of_bounds_temp"].get<bool>());

    auto resp2 = tcu.handle_command(kPoll);
    EXPECT_FALSE(resp2["out_of_bounds_temp"].get<bool>());
}

// Two independent fault fields — each has its own latch
TEST(TemperatureControlUnit, BothFaultsIndependent) {
    TemperatureControlUnit tcu("tcu-test");

    nlohmann::json inj1 = {{"uid","tcu-test"},{"command","inject"},{"inject",{{"overheating",true}}}};
    tcu.handle_command(inj1);
    auto r1 = tcu.handle_command(kPoll);
    EXPECT_TRUE(r1["overheating"].get<bool>());
    EXPECT_FALSE(r1["out_of_bounds_temp"].get<bool>());
    tcu.handle_command(kPoll);  // clear overheating latch

    nlohmann::json inj2 = {{"uid","tcu-test"},{"command","inject"},{"inject",{{"out_of_bounds_temp",true}}}};
    tcu.handle_command(inj2);
    auto r2 = tcu.handle_command(kPoll);
    EXPECT_FALSE(r2["overheating"].get<bool>());
    EXPECT_TRUE(r2["out_of_bounds_temp"].get<bool>());
}

// get_type / get_uid
TEST(TemperatureControlUnit, GetType) {
    TemperatureControlUnit tcu("tcu-test");
    EXPECT_EQ(tcu.get_type(), "TemperatureControlUnit");
    EXPECT_EQ(tcu.get_uid(), "tcu-test");
}
