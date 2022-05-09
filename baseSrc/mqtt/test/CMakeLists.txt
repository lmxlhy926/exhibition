add_executable(subscribe subscribe.cpp)
target_link_libraries(subscribe PRIVATE mqttclient)

add_executable(publish publish.cpp)
target_link_libraries(publish PRIVATE mqttclient)