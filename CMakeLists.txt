cmake_minimum_required(VERSION 3.15)
project(dualventer)

set(CMAKE_CXX_STANDARD 17)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/modules" DESTINATION "${PROJECT_BINARY_DIR}/")

add_executable(dualventer src/main.cpp src/bot.cpp src/context.cpp)
target_include_directories(dualventer PRIVATE include/)
add_subdirectory(lib)