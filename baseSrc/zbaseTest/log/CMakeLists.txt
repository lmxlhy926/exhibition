add_executable(logTest logtest.cpp)
target_link_libraries(logTest PRIVATE log)
target_link_libraries(logTest PRIVATE qlibc)
target_link_libraries(logTest PRIVATE common)