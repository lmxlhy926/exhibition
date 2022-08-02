add_executable(httpClient httpClient.cpp)
target_link_libraries(httpClient PUBLIC qlibc)
target_link_libraries(httpClient PUBLIC common)
target_link_libraries(httpClient PUBLIC log)


