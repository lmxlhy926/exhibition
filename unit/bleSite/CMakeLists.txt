FILE(GLOB src "*.cpp")

add_subdirectory(serial)
add_subdirectory(formatTrans)
add_subdirectory(logic)
add_subdirectory(service)


add_executable(ble_site ${src})
target_link_libraries(ble_site PRIVATE qlibc)
target_link_libraries(ble_site PRIVATE siteService)
target_link_libraries(ble_site PRIVATE common)
target_link_libraries(ble_site PRIVATE format)
target_link_libraries(ble_site PRIVATE serial)
target_link_libraries(ble_site PRIVATE log)
target_link_libraries(ble_site PRIVATE bleLogic)
target_link_libraries(ble_site PRIVATE bleService)