cmake_minimum_required(VERSION 3.20)
project(FaultDetectionSystem CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Codegen pre-build step ----
find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_custom_target(run_codegen ALL
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/codegen/codegen.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running message + fault ID codegen"
    VERBATIM
)

# ---- Third-party dependencies via FetchContent ----
include(FetchContent)

FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.14.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
    GIT_SHALLOW    TRUE
)
# Prevent GoogleTest from overriding compiler/linker options for MSVC
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_subdirectory(monitor)
add_subdirectory(device_simulator)
