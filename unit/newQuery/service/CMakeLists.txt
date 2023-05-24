
add_library(service STATIC serviceRequestHandler.cpp)
target_include_directories(service PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(service PRIVATE http)
target_link_libraries(service PRIVATE common)
target_link_libraries(service PRIVATE qlibc)
target_link_libraries(service PRIVATE log)
target_link_libraries(service PRIVATE mdns)