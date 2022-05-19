cmake_minimum_required(VERSION 3.19)
project(exhibition)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/output" CACHE STRING "The path to use for make install" FORCE)

if(CYGWIN)
    message("sssss")
endif()



add_subdirectory(baseSrc)
add_subdirectory(unit)

#install(TARGETS aes         DESTINATION static_lib)
#install(TARGETS common      DESTINATION static_lib)
#install(TARGETS encrypt     DESTINATION static_lib)
#install(TARGETS mqttclient  DESTINATION static_lib)
#install(TARGETS qlibc       DESTINATION static_lib)
#install(TARGETS siteService DESTINATION static_lib)
#install(TARGETS socket      DESTINATION static_lib)

install(TARGETS aes_shared          DESTINATION shared_lib)
install(TARGETS common_shared       DESTINATION shared_lib)
install(TARGETS encrypt_shared      DESTINATION shared_lib)
install(TARGETS qlibc_shared        DESTINATION shared_lib)
install(TARGETS mqttclient_shared   DESTINATION shared_lib)
install(TARGETS siteService_shared  DESTINATION shared_lib)
install(TARGETS socket_shared       DESTINATION shared_lib)

install(TARGETS config_shared      DESTINATION shared_lib)



#install(TARGETS config      DESTINATION bin)
#install(TARGETS synergy     DESTINATION bin)
#install(TARGETS testSite    DESTINATION bin)
















