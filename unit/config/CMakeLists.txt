FILE(GLOB src "*.cpp")

add_subdirectory(util)

add_executable(config_site  ${src})
target_link_libraries(config_site PRIVATE configUtil)
target_link_libraries(config_site PRIVATE log)

add_subdirectory(test)