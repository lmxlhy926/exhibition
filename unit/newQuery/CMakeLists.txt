
add_subdirectory(mdns)
add_executable(newQuery_site mdns.c)
target_link_libraries(newQuery_site PRIVATE mdns)