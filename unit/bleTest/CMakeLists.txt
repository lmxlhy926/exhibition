FILE(GLOB src "*.cpp")

add_executable(bleTest ${src})
target_link_libraries(bleTest PRIVATE qlibc)
target_link_libraries(bleTest PRIVATE common)
target_link_libraries(bleTest PRIVATE log)
