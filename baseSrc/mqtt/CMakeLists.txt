add_library(mqttclient STATIC mqttClient.cpp mqttMessageHandler.cpp)
target_include_directories(mqttclient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

if(CYGWIN)
    target_link_libraries(mqttclient PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/mqtt/lib/cygwin/libpaho-mqtt3a.a)
elseif(ANDROID)
    target_link_libraries(mqttclient PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/mqtt/lib/android/libpaho-mqtt3a.a)
else()
    target_link_libraries(mqttclient PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/mqtt/lib/linux/libpaho-mqtt3a.a)
    target_link_libraries(mqttclient PUBLIC pthread)
endif()

target_link_libraries(mqttclient PUBLIC qlibc)

add_subdirectory(test)
