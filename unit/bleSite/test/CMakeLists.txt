add_executable(bletest main.cpp)
target_link_libraries(bletest PRIVATE qlibc)
target_link_libraries(bletest PRIVATE format)
