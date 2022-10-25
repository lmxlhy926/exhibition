FILE(GLOB src "*.cpp")

add_library(ssl STATIC ${src})
target_link_libraries(ssl PRIVATE qlibc)
target_link_libraries(ssl PRIVATE common)
target_link_libraries(ssl PRIVATE log)