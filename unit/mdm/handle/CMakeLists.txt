FILE(GLOB src "*.cpp")

add_library(handle  STATIC ${src})
target_include_directories(handle PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(handle PRIVATE qlibc)
target_link_libraries(handle PRIVATE log)