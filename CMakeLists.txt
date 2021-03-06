cmake_minimum_required(VERSION 3.19)
project(exhibition)
set(CMAKE_CXX_STANDARD 14)

#设置可执行程序安装路径
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/out" CACHE STRING "The path to use for make install" FORCE)

if(CYGWIN)
    set(outDir cygwin)
    message("===>CYGWIN")
elseif(ANDROID)
    set(outDir android)
    message("===>android")
endif()

set(libType "STATIC")
message("===>libType: ${libType}")
set(TEST_ENABLE true)

add_subdirectory(baseSrc)
add_subdirectory(unit)

if("${libType}" STREQUAL "SHARED")
    install(TARGETS aes             DESTINATION ${outDir})
    install(TARGETS common          DESTINATION ${outDir})
    install(TARGETS encrypt         DESTINATION ${outDir})
    install(TARGETS qlibc           DESTINATION ${outDir})
    install(TARGETS mqttclient      DESTINATION ${outDir})
    install(TARGETS siteService     DESTINATION ${outDir})
    install(TARGETS socket          DESTINATION ${outDir})
    install(TARGETS config_site     DESTINATION ${outDir})
    install(TARGETS synergy_site    DESTINATION ${outDir})
    if(CYGWIN)
        install(FILES baseSrc/mqtt/lib/cygwin/libpaho-mqtt3a.dll.a
                DESTINATION ${outDir})
    elseif(ANDROID)
        install(FILES baseSrc/mqtt/lib/android/libpaho-mqtt3a.so
                DESTINATION ${outDir})
    endif()

elseif("${libType}" STREQUAL "STATIC")
    install(TARGETS config_site             DESTINATION bin)
    install(TARGETS synergy_site            DESTINATION bin)
    install(TARGETS whiteList_site          DESTINATION bin)
endif()















