#include <gtest/gtest.h>
#include "GarageDoor.hpp"

// Normal poll — all faults false
TEST(GarageDoor, NormalResponseFaultsFalse) {
    GarageDoor gd("gd-test");
    nlohmann::json cmd = {{"uid","gd-test"},{"command","poll"},{"value",0}};
    auto resp = gd.handle_command(cmd);
    EXPECT_FALSE(resp["garage_open"].get<bool>());
    EXPECT_FALSE(resp["light_on"].get<bool>());
    EXPECT_FALSE(resp["over_voltage"].get<bool>());
    EXPECT_EQ(resp["uid"].get<std::string>(), "gd-test");
}

// Each fault field can be injected independently and latches reset
TEST(GarageDoor, InjectGarageOpenFiresOnce) {
    GarageDoor gd("gd-test");
    nlohmann::json inject = {{"uid","gd-test"},{"command","poll"},{"value",0},
                              {"inject",{{"garage_open",true}}}};
    auto r1 = gd.handle_command(inject);
    EXPECT_TRUE(r1["garage_open"].get<bool>());
    EXPECT_FALSE(r1["light_on"].get<bool>());
    EXPECT_FALSE(r1["over_voltage"].get<bool>());

    nlohmann::json normal = {{"uid","gd-test"},{"command","poll"},{"value",0}};
    auto r2 = gd.handle_command(normal);
    EXPECT_FALSE(r2["garage_open"].get<bool>());
}

TEST(GarageDoor, InjectLightOnFiresOnce) {
    GarageDoor gd("gd-test");
    nlohmann::json inject = {{"uid","gd-test"},{"command","poll"},{"value",0},
                              {"inject",{{"light_on",true}}}};
    auto r1 = gd.handle_command(inject);
    EXPECT_TRUE(r1["light_on"].get<bool>());

    nlohmann::json normal = {{"uid","gd-test"},{"command","poll"},{"value",0}};
    auto r2 = gd.handle_command(normal);
    EXPECT_FALSE(r2["light_on"].get<bool>());
}

TEST(GarageDoor, InjectOverVoltageFiresOnce) {
    GarageDoor gd("gd-test");
    nlohmann::json inject = {{"uid","gd-test"},{"command","poll"},{"value",0},
                              {"inject",{{"over_voltage",true}}}};
    auto r1 = gd.handle_command(inject);
    EXPECT_TRUE(r1["over_voltage"].get<bool>());

    nlohmann::json normal = {{"uid","gd-test"},{"command","poll"},{"value",0}};
    auto r2 = gd.handle_command(normal);
    EXPECT_FALSE(r2["over_voltage"].get<bool>());
}

TEST(GarageDoor, GetType) {
    GarageDoor gd("gd-test");
    EXPECT_EQ(gd.get_type(), "GarageDoor");
    EXPECT_EQ(gd.get_uid(), "gd-test");
}
