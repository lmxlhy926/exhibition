
set(src
    config.cpp
    myutil.cpp
    cloudUtil.cpp
    serviceRequestHandler.cpp
    subscribeMessageHandler.cpp
    mqttPayloadHandle.cpp)

add_executable(config  ${src})
target_include_directories(config PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(config PRIVATE encrypt)
target_link_libraries(config PRIVATE qlibc)
target_link_libraries(config PRIVATE siteService)
target_link_libraries(config PRIVATE socket)
target_link_libraries(config PRIVATE aes)
target_link_libraries(config PRIVATE common)
target_link_libraries(config PRIVATE mqttclient)


add_subdirectory(test)