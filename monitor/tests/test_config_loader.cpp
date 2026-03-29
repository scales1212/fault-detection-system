#include <gtest/gtest.h>
#include "ConfigLoader.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

static void write_temp_config(const std::string& path, const std::string& content) {
    std::ofstream f(path);
    f << content;
}

class ConfigLoaderTest : public ::testing::Test {
protected:
    std::string tmp_ = "test_config_tmp.json";
    void TearDown() override { fs::remove(tmp_); }
};

TEST_F(ConfigLoaderTest, ParsesValidConfig) {
    write_temp_config(tmp_, R"({
        "devices": [
            {"uid":"tcu-01","type":"TemperatureControlUnit","port":9001},
            {"uid":"gd-01","type":"GarageDoor","port":9003}
        ]
    })");
    ConfigLoader loader(tmp_);
    loader.load();
    const auto& devs = loader.get_devices();
    ASSERT_EQ(devs.size(), 2u);
    EXPECT_EQ(devs[0].uid,  "tcu-01");
    EXPECT_EQ(devs[0].type, "TemperatureControlUnit");
    EXPECT_EQ(devs[0].port, 9001u);
    EXPECT_EQ(devs[1].uid,  "gd-01");
}

TEST_F(ConfigLoaderTest, ThrowsOnMissingFile) {
    ConfigLoader loader("nonexistent.json");
    EXPECT_THROW(loader.load(), std::runtime_error);
}

TEST_F(ConfigLoaderTest, ThrowsOnMissingDevicesArray) {
    write_temp_config(tmp_, R"({"foo": []})");
    ConfigLoader loader(tmp_);
    EXPECT_THROW(loader.load(), std::runtime_error);
}

TEST_F(ConfigLoaderTest, ThrowsOnMissingField) {
    write_temp_config(tmp_, R"({"devices": [{"uid":"x","type":"T"}]})");
    ConfigLoader loader(tmp_);
    EXPECT_THROW(loader.load(), std::runtime_error);
}
