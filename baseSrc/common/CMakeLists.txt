
add_library(common STATIC httpUtil.cpp httplib.cc)
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(common PRIVATE qlibc)
#target_link_libraries(common PRIVATE tsl)

if(NOT ANDROID)
    target_link_libraries(common PRIVATE pthread)
endif()

