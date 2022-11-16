
add_subdirectory(mdns)

add_executable(newQuery_site mm.c)
target_link_libraries(newQuery_site PUBLIC mdnsBasic)