#include <gtest/gtest.h>
#include "DeviceProxy.hpp"
#include "FaultIds.hpp"
#include <algorithm>
#include <string>
#include <vector>

// Verify fault_fields_ is populated from the registry (not hard-coded)
TEST(DeviceProxy, FaultFieldsFromRegistry) {
    // We can't easily construct a DeviceProxy (it opens a socket),
    // so we test the registry directly here as a proxy for the integration.
    const auto& reg = getFaultIdRegistry();

    std::vector<std::string> tcu_fields;
    for (const auto& [key, fid] : reg)
        if (fid.device_type == "TemperatureControlUnit")
            tcu_fields.push_back(fid.field);

    EXPECT_NE(std::find(tcu_fields.begin(), tcu_fields.end(), "overheating"),
              tcu_fields.end());
    EXPECT_NE(std::find(tcu_fields.begin(), tcu_fields.end(), "out_of_bounds_temp"),
              tcu_fields.end());
}

TEST(DeviceProxy, GarageDoorFaultFieldsFromRegistry) {
    const auto& reg = getFaultIdRegistry();

    std::vector<std::string> gd_fields;
    for (const auto& [key, fid] : reg)
        if (fid.device_type == "GarageDoor")
            gd_fields.push_back(fid.field);

    EXPECT_NE(std::find(gd_fields.begin(), gd_fields.end(), "garage_open"),  gd_fields.end());
    EXPECT_NE(std::find(gd_fields.begin(), gd_fields.end(), "light_on"),     gd_fields.end());
    EXPECT_NE(std::find(gd_fields.begin(), gd_fields.end(), "over_voltage"), gd_fields.end());
}

// Verify numeric IDs are correct in the registry
TEST(DeviceProxy, FaultIdNumericIds) {
    const auto& reg = getFaultIdRegistry();
    auto it = reg.find({"TemperatureControlUnit", "overheating"});
    ASSERT_NE(it, reg.end());
    EXPECT_EQ(it->second.id, "TCU_OVERHEAT");
    EXPECT_EQ(it->second.numeric_id, 1);
}
