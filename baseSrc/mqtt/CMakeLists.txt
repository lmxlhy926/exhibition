add_library(mqttclient ${libType} mqttClient.cpp mqttMessageHandler.cpp)
target_include_directories(mqttclient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

if(CYGWIN)
    if("${libType}" STREQUAL "STATIC")
        target_link_libraries(mqttclient PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/mqtt/lib/cygwin/libpaho-mqtt3a.a)
    elseif("${libType}" STREQUAL "SHARED")
        target_link_libraries(mqttclient PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/mqtt/lib/cygwin/libpaho-mqtt3a.dll.a)
    endif()

elseif(ANDROID)
    if("${libType}" STREQUAL "STATIC")
        target_link_libraries(mqttclient PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/mqtt/lib/android/libpaho-mqtt3a.a)
    elseif("${libType}" STREQUAL "SHARED")
        target_link_libraries(mqttclient PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/mqtt/lib/android/libpaho-mqtt3a.so)
    endif()
endif()

target_link_libraries(mqttclient PUBLIC qlibc)

add_subdirectory(test)
