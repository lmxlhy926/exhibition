add_executable(mqtttest test.cpp)
target_link_libraries(mqtttest PRIVATE mqttclient)