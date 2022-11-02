add_executable(http_test httpRequest.cpp)
target_link_libraries(http_test PRIVATE qlibc)
target_link_libraries(http_test PRIVATE common)
target_link_libraries(http_test PRIVATE log)

add_executable(httpServer_test httpServer.cpp)
target_link_libraries(httpServer_test PRIVATE qlibc)
target_link_libraries(httpServer_test PRIVATE common)
target_link_libraries(httpServer_test PRIVATE log)