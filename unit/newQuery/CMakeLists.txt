
add_subdirectory(siteManage)
add_subdirectory(mdns)
add_subdirectory(service)

add_executable(query_site main.cpp)
target_link_libraries(query_site PRIVATE qlibc)
target_link_libraries(query_site PRIVATE siteService)
target_link_libraries(query_site PRIVATE common)
target_link_libraries(query_site PRIVATE log)
target_link_libraries(query_site PRIVATE service)