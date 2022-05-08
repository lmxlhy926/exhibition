add_library(mqttclient SHARED mqttClient.cpp mqttMessageHandler.cpp)
target_include_directories(mqttclient PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
#target_link_libraries(mqttclient PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/mqtt/lib/libpaho-mqtt3a.dll.a)
target_link_libraries(mqttclient PUBLIC qlibc)

add_subdirectory(test)