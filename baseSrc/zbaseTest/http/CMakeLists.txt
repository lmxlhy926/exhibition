add_executable(http_test httpRequest.cpp)
target_link_libraries(http_test PRIVATE qlibc)
target_link_libraries(http_test PRIVATE common)
target_link_libraries(http_test PRIVATE log)