add_executable(qlibc_test test.cpp)
target_include_directories(qlibc_test PRIVATE ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(qlibc_test PRIVATE qlibc)




