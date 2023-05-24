
add_library(siteManage STATIC siteManageUtil.cpp)
target_include_directories(siteManage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(siteManage PRIVATE http)
target_link_libraries(siteManage PUBLIC qlibc)
target_link_libraries(siteManage PRIVATE log)
target_link_libraries(siteManage PRIVATE siteService)
target_link_libraries(siteManage PRIVATE mdns)
target_link_libraries(siteManage PRIVATE common)

