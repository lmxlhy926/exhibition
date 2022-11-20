add_library(mdnsBasic STATIC mdns.c mdnsUtil.cpp)
target_include_directories(mdnsBasic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(mdnsBasic PRIVATE siteManage)
target_link_libraries(mdnsBasic PRIVATE log)