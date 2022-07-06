FILE(GLOB src "*.cpp")

add_executable(whiteList_site  ${src})
target_include_directories(whiteList_site PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(whiteList_site PRIVATE qlibc)
target_link_libraries(whiteList_site PRIVATE siteService)
target_link_libraries(whiteList_site PRIVATE common)
