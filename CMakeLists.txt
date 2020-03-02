cmake_minimum_required(VERSION 3.15)
project(dualventer)

set(CMAKE_CXX_STANDARD 17)

add_executable(dualventer src/main.cpp src/bot.cpp src/snowflake.cpp)
target_include_directories(dualventer PRIVATE include/)
add_subdirectory(lib)