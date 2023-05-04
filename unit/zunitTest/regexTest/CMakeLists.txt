add_executable(regexTest main.cpp)
target_link_libraries(regexTest PRIVATE qlibc)
target_link_libraries(regexTest PRIVATE log)