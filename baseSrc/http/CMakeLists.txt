
add_library(http STATIC httplib.cc)
target_include_directories(http PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

#增加ssl支持
#target_include_directories(http PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tsl/include)
#target_link_libraries(http PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tsl/libcrypto.a)
#target_link_libraries(http PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tsl/libssl.a)
#target_link_libraries(http PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tsl/libcrypto.so.1.1)
#target_link_libraries(http PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tsl/libssl.so.1.1)


if(NOT ANDROID)
    target_link_libraries(http PRIVATE pthread)
endif()
