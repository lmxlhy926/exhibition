
add_subdirectory(mdns)
add_subdirectory(sitemanager)

add_executable(sitemgs main.cpp)
target_link_libraries(sitemgs sitemg mdns)
