add_executable(serialTest listhandle.cpp)
target_link_libraries(serialTest PUBLIC qlibc)
target_link_libraries(serialTest PUBLIC format)