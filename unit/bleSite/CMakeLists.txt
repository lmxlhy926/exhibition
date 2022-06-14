
FILE(GLOB mainSrc "*.cpp")
FILE(GLOB subSrc "formatTrans/*.cpp")
FILE(GLOB kernalSrc "kernal/*.cpp")
set(src ${mainSrc} ${subSrc} ${kernalSrc})

add_executable(ble_site ${src})
target_link_libraries(ble_site PRIVATE qlibc)
target_link_libraries(ble_site PRIVATE siteService)
target_link_libraries(ble_site PRIVATE socket)
target_link_libraries(ble_site PRIVATE common)

add_subdirectory(test)