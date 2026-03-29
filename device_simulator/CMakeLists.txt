add_executable(device_simulator
    src/main.cpp
    src/DeviceSimulator.cpp
    src/TemperatureControlUnit.cpp
    src/GarageDoor.cpp
    src/UdpSocket.cpp
)

add_dependencies(device_simulator run_codegen)

target_include_directories(device_simulator PRIVATE
    src
    ${CMAKE_SOURCE_DIR}/generated/cpp
)

target_link_libraries(device_simulator PRIVATE
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    ws2_32
)

# ---- Tests ----
add_executable(device_simulator_tests
    tests/test_temperature_control_unit.cpp
    tests/test_garage_door.cpp
    src/TemperatureControlUnit.cpp
    src/GarageDoor.cpp
)

add_dependencies(device_simulator_tests run_codegen)

target_include_directories(device_simulator_tests PRIVATE
    src
    ${CMAKE_SOURCE_DIR}/generated/cpp
)

target_link_libraries(device_simulator_tests PRIVATE
    GTest::gtest_main
    nlohmann_json::nlohmann_json
)

include(GoogleTest)
gtest_discover_tests(device_simulator_tests)
