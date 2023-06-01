cmake_minimum_required(VERSION 3.14)
project(exhibition)
set(CMAKE_CXX_STANDARD 14)

if(CYGWIN_X86_64)
    message("CYGWIN_X86_64--------")
elseif(LINUX_X86_64)
    message("LINUX_X86_64--------")
elseif(ANDROID_ARM32)
    message("ANDROID_ARM32--------")
elseif(LINUX_ARM64)
    message("LINUX_ARM64--------")
endif()


#设置可执行程序安装路径
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/out" CACHE STRING "The path to use for make install" FORCE)
set(TEST_ENABLE true)

add_subdirectory(baseSrc)
add_subdirectory(unit)

#install
if(LINUX_X86_64)
    install(TARGETS ble_site                DESTINATION X86_64)
    install(TARGETS config_site             DESTINATION X86_64)
    install(TARGETS synergy_site            DESTINATION X86_64)
    install(TARGETS query_site              DESTINATION X86_64)
    install(TARGETS mdns                    DESTINATION X86_64)
elseif(ANDROID_ARM32)
    install(TARGETS ble_site                DESTINATION android32)
    install(TARGETS config_site             DESTINATION android32)
    install(TARGETS synergy_site            DESTINATION android32)
    install(TARGETS query_site              DESTINATION android32)
    install(TARGETS mdns                    DESTINATION android32)
elseif(LINUX_ARM64)
    install(TARGETS ble_site                DESTINATION arm64)
    install(TARGETS config_site             DESTINATION arm64)
    install(TARGETS synergy_site            DESTINATION arm64)
    install(TARGETS query_site              DESTINATION arm64)
    install(TARGETS mdns                    DESTINATION arm64)
endif()


