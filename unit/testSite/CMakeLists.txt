FILE(GLOB src "*.cpp")

add_executable(test_site  ${src})
target_include_directories(test_site PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(test_site PRIVATE qlibc)
target_link_libraries(test_site PRIVATE siteService)
target_link_libraries(test_site PRIVATE common)
