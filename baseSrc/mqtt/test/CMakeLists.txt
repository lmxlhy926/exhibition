
add_executable(mqtt_subscribe_test subscribe.cpp)
target_link_libraries(mqtt_subscribe_test PRIVATE mqttclient)
target_link_libraries(mqtt_subscribe_test PRIVATE qlibc)

add_executable(mqtt_publish_test publish.cpp)
target_link_libraries(mqtt_publish_test PRIVATE mqttclient)
target_link_libraries(mqtt_publish_test PRIVATE qlibc)

