
FILE(GLOB mainSrc "*.cpp")
FILE(GLOB kernalSrc "kernal/*.cpp")
set(src ${mainSrc} ${kernalSrc})

add_subdirectory(formatTrans)
add_subdirectory(serial)

add_executable(ble_site ${src})
target_link_libraries(ble_site PRIVATE qlibc)
target_link_libraries(ble_site PRIVATE siteService)
target_link_libraries(ble_site PRIVATE socket)
target_link_libraries(ble_site PRIVATE common)
target_link_libraries(ble_site PRIVATE format)
target_link_libraries(ble_site PRIVATE serial)

add_subdirectory(test)
