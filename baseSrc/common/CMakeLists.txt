
add_library(common STATIC httpUtil.cpp httplib.cc)
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(common PUBLIC qlibc)
target_link_libraries(common PUBLIC pthread)

