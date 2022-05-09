add_library(mqttclient STATIC mqttClient.cpp mqttMessageHandler.cpp)
target_include_directories(mqttclient PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(mqttclient PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/mqtt/lib/libpaho-mqtt3a.a)
target_link_libraries(mqttclient PUBLIC qlibc)

add_subdirectory(test)