cmake_minimum_required(VERSION 3.19)
project(exhibition)
set(CMAKE_CXX_STANDARD 14)

if(CYGWIN)
    message("====>CYGWIN.....")
elseif(ANDROID)
    message("====>ANDROID.....")
else()
    message("====>LINUX.....")
endif()

#设置可执行程序安装路径
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/out" CACHE STRING "The path to use for make install" FORCE)
set(TEST_ENABLE true)

add_subdirectory(baseSrc)
add_subdirectory(unit)

#install
install(TARGETS config_site             DESTINATION bin)
#install(TARGETS synergy_site            DESTINATION bin)
install(TARGETS whiteList_site          DESTINATION bin)
install(TARGETS whiteListPublish        DESTINATION bin)
install(TARGETS whiteListScribe         DESTINATION bin)
install(TARGETS mdns_test               DESTINATION bin)
install(TARGETS query_site              DESTINATION bin)
install(TARGETS httpClient              DESTINATION bin)
install(TARGETS ble_site                DESTINATION bin)


















