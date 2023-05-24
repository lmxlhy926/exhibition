
add_library(mdns INTERFACE)
target_include_directories(mdns INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(LINUX_X86_64)
    target_link_libraries(mdns INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/libmdns.a)
elseif(ANDROID_ARM32)
    target_link_libraries(mdns INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lib/android/libmdns.a)
elseif(LINUX_ARM64)
    target_link_libraries(mdns INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lib/arm64/libmdns.a)
endif()
# target_link_libraries(mdns INTERFACE siteManage)
# target_link_libraries(mdns INTERFACE log)

