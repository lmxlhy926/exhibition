FILE(GLOB src "*.cpp")

add_library(configUtil STATIC ${src})
target_link_libraries(configUtil PUBLIC encrypt)
target_link_libraries(configUtil PUBLIC qlibc)
target_link_libraries(configUtil PUBLIC aes)
target_link_libraries(configUtil PUBLIC common)
target_link_libraries(configUtil PUBLIC mqttclient)
target_link_libraries(configUtil PUBLIC siteService)


