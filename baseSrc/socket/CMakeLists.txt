FILE(GLOB src "*.cpp")

add_library(socket STATIC ${src})
target_include_directories(socket PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(socket PUBLIC qlibc)
target_link_libraries(socket PUBLIC common)

add_subdirectory(test)
