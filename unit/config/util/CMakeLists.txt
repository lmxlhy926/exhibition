FILE(GLOB src "*.cpp")

add_library(configUtil STATIC ${src})
target_link_libraries(configUtil PUBLIC encrypt)
target_link_libraries(configUtil PUBLIC qlibc)
target_link_libraries(configUtil PUBLIC aes)
target_link_libraries(configUtil PUBLIC common)
target_link_libraries(configUtil PUBLIC mqttclient)
target_link_libraries(configUtil PUBLIC siteService)
target_link_libraries(configUtil PUBLIC log)
target_include_directories(configUtil PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)


