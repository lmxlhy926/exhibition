add_executable(serialTest serialTest.cpp)
target_link_libraries(serialTest PRIVATE qlibc)
target_link_libraries(serialTest PRIVATE format)
target_link_libraries(serialTest PRIVATE serial)

add_executable(httpRequest httpRequest.cpp)
target_link_libraries(httpRequest PRIVATE qlibc)
target_link_libraries(httpRequest PRIVATE format)
target_link_libraries(httpRequest PRIVATE serial)
target_link_libraries(httpRequest PRIVATE socket)


