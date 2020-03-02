target_include_directories(dualventer PRIVATE ${CMAKE_REQUIRED_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/asio/asio/include
        ${CMAKE_CURRENT_SOURCE_DIR}/sol2/single/include
        ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include)

add_subdirectory(cpr)

set(USE_STANDALONE_ASIO ON)
add_subdirectory(sws)

target_link_libraries(dualventer PRIVATE ${CPR_LIBRARIES} simple-websocket-server lua)
target_compile_definitions(dualventer PRIVATE ASIO_STANDALONE)