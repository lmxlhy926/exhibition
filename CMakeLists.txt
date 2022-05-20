cmake_minimum_required(VERSION 3.19)
project(exhibition)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/out" CACHE STRING "The path to use for make install" FORCE)

add_subdirectory(baseSrc)
add_subdirectory(unit)

if(CYGWIN)
    set(outDir cygwin)
elseif(ANDROID)
    set(outDir android)
endif()

install(TARGETS aes_shared          DESTINATION ${outDir})
install(TARGETS common_shared       DESTINATION ${outDir})
install(TARGETS encrypt_shared      DESTINATION ${outDir})
install(TARGETS qlibc_shared        DESTINATION ${outDir})
install(TARGETS mqttclient_shared   DESTINATION ${outDir})
install(TARGETS siteService_shared  DESTINATION ${outDir})
install(TARGETS socket_shared       DESTINATION ${outDir})
install(TARGETS config_shared      DESTINATION ${outDir})

install(TARGETS config      DESTINATION bin)


#install(TARGETS synergy     DESTINATION bin)
#install(TARGETS testSite    DESTINATION bin)
















