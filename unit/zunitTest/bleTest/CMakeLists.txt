
add_executable(ble_test main.cpp)
target_link_libraries(ble_test PRIVATE qlibc)
target_link_libraries(ble_test PRIVATE common)
target_link_libraries(ble_test PRIVATE log)


