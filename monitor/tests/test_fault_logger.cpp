#include <gtest/gtest.h>
#include "FaultLogger.hpp"
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

static std::string read_file(const std::string& path) {
    std::ifstream f(path);
    return std::string(std::istreambuf_iterator<char>(f), {});
}

class FaultLoggerTest : public ::testing::Test {
protected:
    std::string log_path_ = "test_fault_logger_output.log";

    void TearDown() override {
        fs::remove(log_path_);
    }
};

TEST_F(FaultLoggerTest, SendEntryWritten) {
    FaultLogger logger(log_path_);
    logger.log_send("tcu-01", R"({"uid":"tcu-01","command":"poll"})");
    auto contents = read_file(log_path_);
    EXPECT_NE(contents.find("[SEND]"), std::string::npos);
    EXPECT_NE(contents.find("tcu-01"), std::string::npos);
}

TEST_F(FaultLoggerTest, ReceiveEntryWritten) {
    FaultLogger logger(log_path_);
    logger.log_receive("tcu-01", R"({"uid":"tcu-01","overheating":false})");
    auto contents = read_file(log_path_);
    EXPECT_NE(contents.find("[RECV]"), std::string::npos);
}

TEST_F(FaultLoggerTest, TimeoutEntryWritten) {
    FaultLogger logger(log_path_);
    logger.log_timeout("tcu-01");
    auto contents = read_file(log_path_);
    EXPECT_NE(contents.find("[TIMEOUT]"), std::string::npos);
}

TEST_F(FaultLoggerTest, FaultFailEntryContainsKeywords) {
    FaultLogger logger(log_path_);
    FaultId fid{"TCU_OVERHEAT", 1, "TemperatureControlUnit", "overheating", {"temperature","safety"}};
    logger.log_fault_transition("tcu-01", fid, FaultState::FAIL);
    auto contents = read_file(log_path_);
    EXPECT_NE(contents.find("[FAULT:FAIL]"),  std::string::npos);
    EXPECT_NE(contents.find("TCU_OVERHEAT"), std::string::npos);
    EXPECT_NE(contents.find("temperature"),  std::string::npos);
}

TEST_F(FaultLoggerTest, FaultPassEntryContainsKeywords) {
    FaultLogger logger(log_path_);
    FaultId fid{"TCU_OVERHEAT", 1, "TemperatureControlUnit", "overheating", {"temperature"}};
    logger.log_fault_transition("tcu-01", fid, FaultState::PASS);
    auto contents = read_file(log_path_);
    EXPECT_NE(contents.find("[FAULT:PASS]"), std::string::npos);
}

TEST_F(FaultLoggerTest, GroupFailEntryWritten) {
    FaultLogger logger(log_path_);
    logger.log_group_transition("safety", FaultState::FAIL);
    auto contents = read_file(log_path_);
    EXPECT_NE(contents.find("[GROUP:FAIL]"), std::string::npos);
    EXPECT_NE(contents.find("safety"),       std::string::npos);
}

TEST_F(FaultLoggerTest, GroupPassEntryWritten) {
    FaultLogger logger(log_path_);
    logger.log_group_transition("temperature", FaultState::PASS);
    auto contents = read_file(log_path_);
    EXPECT_NE(contents.find("[GROUP:PASS]"),  std::string::npos);
    EXPECT_NE(contents.find("temperature"),   std::string::npos);
}
