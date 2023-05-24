
add_library(spdlog INTERFACE)
target_include_directories(spdlog INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(LINUX_X86_64)
    target_link_libraries(spdlog INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/libspdlogd.a)
    target_link_libraries(spdlog INTERFACE pthread)
elseif(ANDROID_ARM32)
    target_link_libraries(spdlog INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lib/android/libspdlog.a)
elseif(LINUX_ARM64)
    target_link_libraries(spdlog INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lib/arm64/libspdlog.a)
    target_link_libraries(spdlog INTERFACE pthread)
endif()

