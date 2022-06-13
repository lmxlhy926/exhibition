FILE(GLOB src "*.cpp")

add_executable(config_site  ${src})
target_include_directories(config_site PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(config_site PRIVATE encrypt)
target_link_libraries(config_site PRIVATE qlibc)
target_link_libraries(config_site PRIVATE siteService)
target_link_libraries(config_site PRIVATE socket)
target_link_libraries(config_site PRIVATE aes)
target_link_libraries(config_site PRIVATE common)
target_link_libraries(config_site PRIVATE mqttclient)


#add_subdirectory(test)