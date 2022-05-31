add_executable(qlibc_test test.cpp)
target_include_directories(qlibc_test PRIVATE ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(qlibc_test PRIVATE qlibc)


add_executable(log_test logtest.cpp)
target_include_directories(log_test PRIVATE ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(log_test PRIVATE qlibc)

