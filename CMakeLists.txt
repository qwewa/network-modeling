cmake_minimum_required(VERSION 3.16)
project(DistributedScheduler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===== JSON =====
include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)

FetchContent_MakeAvailable(nlohmann_json)

# ===== LIBRARY =====
add_library(core
    lib/Data.cpp
    lib/Graph.cpp
    lib/Resource.cpp
    lib/Scheduler.cpp
    lib/Worker.cpp
)

target_include_directories(core PUBLIC lib)

# Линкуем json
target_link_libraries(core PUBLIC nlohmann_json::nlohmann_json)

# ===== EXECUTABLE =====
add_executable(main src/main.cpp)

target_link_libraries(main PRIVATE core)