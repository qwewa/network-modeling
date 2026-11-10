cmake_minimum_required(VERSION 3.16)
project(DistributedScheduler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===== OPENMP FOR macOS (Homebrew) =====
if(APPLE)
    set(OpenMP_ROOT "/opt/homebrew/opt/libomp")
endif()

# ===== DEPENDENCIES =====
find_package(OpenMP REQUIRED)
find_package(nlohmann_json REQUIRED)

# ===== LIBRARY =====
add_library(core
    lib/Data.cpp
    lib/Graph.cpp
    lib/Resource.cpp
    lib/Scheduler.cpp
    lib/Worker.cpp
    src/openmp.cpp
)

target_include_directories(core PUBLIC lib)

# Линкуем библиотеки
target_link_libraries(core PUBLIC nlohmann_json::nlohmann_json OpenMP::OpenMP_CXX)

# ===== EXECUTABLE =====
add_executable(main src/main.cpp)

target_link_libraries(main PRIVATE core)