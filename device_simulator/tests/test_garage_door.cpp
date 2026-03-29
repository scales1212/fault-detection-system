#include <gtest/gtest.h>
#include "GarageDoor.hpp"

static const nlohmann::json kPoll = {{"uid","gd-test"},{"command","poll"},{"value",0}};

// Normal poll — all faults false
TEST(GarageDoor, NormalResponseFaultsFalse) {
    GarageDoor gd("gd-test");
    auto resp = gd.handle_command(kPoll);
    EXPECT_FALSE(resp["garage_open"].get<bool>());
    EXPECT_FALSE(resp["light_on"].get<bool>());
    EXPECT_FALSE(resp["over_voltage"].get<bool>());
    EXPECT_EQ(resp["uid"].get<std::string>(), "gd-test");
}

// Inject command returns null
TEST(GarageDoor, InjectCommandReturnsNull) {
    GarageDoor gd("gd-test");
    nlohmann::json inject = {{"uid","gd-test"},{"command","inject"},{"inject",{{"garage_open",true}}}};
    auto r = gd.handle_command(inject);
    EXPECT_TRUE(r.is_null());
}

// garage_open latch fires on next poll then clears
TEST(GarageDoor, InjectGarageOpenFiresOnNextPoll) {
    GarageDoor gd("gd-test");
    nlohmann::json inject = {{"uid","gd-test"},{"command","inject"},{"inject",{{"garage_open",true}}}};
    gd.handle_command(inject);

    auto r1 = gd.handle_command(kPoll);
    EXPECT_TRUE(r1["garage_open"].get<bool>());
    EXPECT_FALSE(r1["light_on"].get<bool>());
    EXPECT_FALSE(r1["over_voltage"].get<bool>());

    auto r2 = gd.handle_command(kPoll);
    EXPECT_FALSE(r2["garage_open"].get<bool>());
}

// light_on latch fires on next poll then clears
TEST(GarageDoor, InjectLightOnFiresOnNextPoll) {
    GarageDoor gd("gd-test");
    nlohmann::json inject = {{"uid","gd-test"},{"command","inject"},{"inject",{{"light_on",true}}}};
    gd.handle_command(inject);

    auto r1 = gd.handle_command(kPoll);
    EXPECT_TRUE(r1["light_on"].get<bool>());

    auto r2 = gd.handle_command(kPoll);
    EXPECT_FALSE(r2["light_on"].get<bool>());
}

// over_voltage latch fires on next poll then clears
TEST(GarageDoor, InjectOverVoltageFiresOnNextPoll) {
    GarageDoor gd("gd-test");
    nlohmann::json inject = {{"uid","gd-test"},{"command","inject"},{"inject",{{"over_voltage",true}}}};
    gd.handle_command(inject);

    auto r1 = gd.handle_command(kPoll);
    EXPECT_TRUE(r1["over_voltage"].get<bool>());

    auto r2 = gd.handle_command(kPoll);
    EXPECT_FALSE(r2["over_voltage"].get<bool>());
}

TEST(GarageDoor, GetType) {
    GarageDoor gd("gd-test");
    EXPECT_EQ(gd.get_type(), "GarageDoor");
    EXPECT_EQ(gd.get_uid(), "gd-test");
}
