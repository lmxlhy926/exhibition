
set(src
    config.cpp
    secretUtils.cpp
    cloudUtil.cpp
    serviceRequestHandler.cpp
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

add_executable(config_shared  ${src})
target_include_directories(config_shared PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(config_shared PRIVATE encrypt_shared)
target_link_libraries(config_shared PRIVATE qlibc_shared)
target_link_libraries(config_shared PRIVATE siteService_shared)
target_link_libraries(config_shared PRIVATE socket_shared)
target_link_libraries(config_shared PRIVATE aes_shared)
target_link_libraries(config_shared PRIVATE common_shared)
target_link_libraries(config_shared PRIVATE mqttclient)


add_subdirectory(test)