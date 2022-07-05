
add_library(common ${libType}  httpUtil.cpp httplib.cc)
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(common PUBLIC qlibc)

