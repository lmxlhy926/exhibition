add_library(mdns STATIC mdns.c mdnsUtil.cpp)
target_include_directories(mdns PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(mdns PRIVATE siteManage)
target_link_libraries(mdns PRIVATE log)