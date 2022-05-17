
set(src
    testSite.cpp
    serviceHandler.cpp)

add_executable(testSite  ${src})
target_include_directories(testSite PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(testSite PRIVATE qlibc)
target_link_libraries(testSite PRIVATE siteService)
target_link_libraries(testSite PRIVATE common)