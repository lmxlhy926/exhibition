cmake_minimum_required(VERSION 3.14)
project(exhibition)
set(CMAKE_CXX_STANDARD 14)

if(ANDROID)
    message("====>ANDROID.....")
elseif(CYGWIN)
    message("====>CYGWIN.....")
else()
    message("====>LINUX.....")
endif()

#设置可执行程序安装路径
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/out" CACHE STRING "The path to use for make install" FORCE)
set(TEST_ENABLE true)

add_subdirectory(baseSrc)
add_subdirectory(unit)

#install
install(TARGETS ble_site                DESTINATION bin)
install(TARGETS config_site             DESTINATION bin)
install(TARGETS query_site              DESTINATION bin)
install(TARGETS synergy_site            DESTINATION bin)
install(TARGETS site_subcribe_test      DESTINATION bin)
install(TARGETS newQuery_site           DESTINATION bin)




















