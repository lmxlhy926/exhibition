FILE(GLOB src "*.cpp")

add_library(handle  STATIC ${src})
target_link_libraries(handle PRIVATE qlibc)
target_link_libraries(handle PRIVATE log)