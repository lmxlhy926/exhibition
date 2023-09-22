
add_executable(nightStripTest simulate.cpp)
target_link_libraries(nightStripTest PRIVATE common)
target_link_libraries(nightStripTest PRIVATE qlibc)
target_link_libraries(nightStripTest PRIVATE log)









