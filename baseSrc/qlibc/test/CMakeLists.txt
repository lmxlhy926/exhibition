add_executable(qdatatest test.cpp)
target_include_directories(qdatatest PRIVATE ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(qdatatest PRIVATE qlibc)




