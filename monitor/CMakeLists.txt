add_executable(monitor
    src/main.cpp
    src/Monitor.cpp
    src/DeviceProxy.cpp
    src/UdpSocket.cpp
    src/FaultLogger.cpp
    src/FaultStateTracker.cpp
    src/ConfigLoader.cpp
)

add_dependencies(monitor run_codegen)

target_include_directories(monitor PRIVATE
    src
    ${CMAKE_SOURCE_DIR}/generated/cpp
)

target_link_libraries(monitor PRIVATE
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    ws2_32
)

# ---- Tests ----
add_executable(monitor_tests
    tests/test_fault_state_tracker.cpp
    tests/test_fault_logger.cpp
    tests/test_config_loader.cpp
    tests/test_device_proxy.cpp
    src/FaultStateTracker.cpp
    src/FaultLogger.cpp
    src/ConfigLoader.cpp
)

add_dependencies(monitor_tests run_codegen)

target_include_directories(monitor_tests PRIVATE
    src
    ${CMAKE_SOURCE_DIR}/generated/cpp
)

target_link_libraries(monitor_tests PRIVATE
    GTest::gtest_main
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    ws2_32
)

include(GoogleTest)
gtest_discover_tests(monitor_tests)
