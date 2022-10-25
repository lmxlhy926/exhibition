
add_subdirectory(handle)
add_subdirectory(sslUtil)

FILE(GLOB src "*.cpp")
add_executable(mdm_site  ${src})
target_link_libraries(mdm_site PRIVATE qlibc)
target_link_libraries(mdm_site PRIVATE log)
target_link_libraries(mdm_site PRIVATE mqttclient)
target_link_libraries(mdm_site PRIVATE configUtil)
target_link_libraries(mdm_site PRIVATE handle)
target_link_libraries(mdm_site PRIVATE ssl)