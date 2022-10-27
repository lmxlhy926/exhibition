FILE(GLOB src "*.cpp")

add_library(ssl STATIC ${src})
target_include_directories(ssl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(ssl PRIVATE qlibc)
target_link_libraries(ssl PRIVATE log)
target_link_libraries(ssl PRIVATE http)