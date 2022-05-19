add_library(mqttclient STATIC mqttClient.cpp mqttMessageHandler.cpp)
target_include_directories(mqttclient PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
if(CYGWIN)
    target_link_libraries(mqttclient PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/mqtt/lib/cygwin/libpaho-mqtt3a.a)
else()
    target_link_libraries(mqttclient PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/mqtt/lib/android/libpaho-mqtt3a.a)
endif()
target_link_libraries(mqttclient PUBLIC qlibc)


add_library(mqttclient_shared SHARED mqttClient.cpp mqttMessageHandler.cpp)
target_include_directories(mqttclient_shared PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
if(CYGWIN)
    target_link_libraries(mqttclient_shared PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/mqtt/lib/cygwin/libpaho-mqtt3a.dll.a)
else()
    target_link_libraries(mqttclient_shared PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/mqtt/lib/android/libpaho-mqtt3a.so)
endif()
target_link_libraries(mqttclient_shared PUBLIC qlibc_shared)





add_subdirectory(test)