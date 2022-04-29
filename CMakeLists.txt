cmake_minimum_required(VERSION 3.19)
project(exhibition)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(baseSrc)

add_executable(exhibition main.cpp)
target_include_directories(exhibition PRIVATE ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(exhibition PRIVATE qlibc)


