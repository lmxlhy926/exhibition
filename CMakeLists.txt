cmake_minimum_required(VERSION 3.19)
project(exhibition)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/out" CACHE STRING "The path to use for make install" FORCE)

if(CYGWIN)
    set(outDir cygwin)
elseif(ANDROID)
    set(outDir android)
endif()

set(libType "STATIC")
message("===>libType: ${libType}")

add_subdirectory(baseSrc)
add_subdirectory(unit)

if("${libType}" STREQUAL "SHARED")
    install(TARGETS aes          DESTINATION ${outDir})
    install(TARGETS common       DESTINATION ${outDir})
    install(TARGETS encrypt      DESTINATION ${outDir})
    install(TARGETS qlibc        DESTINATION ${outDir})
    install(TARGETS mqttclient   DESTINATION ${outDir})
    install(TARGETS siteService  DESTINATION ${outDir})
    install(TARGETS socket       DESTINATION ${outDir})
    install(TARGETS config       DESTINATION ${outDir})
    install(TARGETS synergy      DESTINATION ${outDir})
    if(CYGWIN)
        install(FILES baseSrc/mqtt/lib/cygwin/libpaho-mqtt3a.dll.a
                DESTINATION ${outDir})
    elseif(ANDROID)
        install(FILES baseSrc/mqtt/lib/android/libpaho-mqtt3a.so
                DESTINATION ${outDir})
    endif()

elseif("${libType}" STREQUAL "STATIC")
    install(TARGETS config       DESTINATION bin)
    install(TARGETS synergy      DESTINATION bin)
endif()

#install(TARGETS testSite    DESTINATION bin)
















