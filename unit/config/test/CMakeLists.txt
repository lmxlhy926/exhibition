
add_executable(configTest test.cpp)
target_include_directories(configTest PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(configTest PUBLIC socket)