
add_subdirectory(mdns)
add_subdirectory(sitemanager)

add_executable(query_site main.cpp)
target_link_libraries(query_site sitemg mdns common qlibc)
