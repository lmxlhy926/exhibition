
add_library(common ${libType} configParamUtil.cpp httpUtil.cpp)
target_link_libraries(common PUBLIC qlibc)
target_link_libraries(common PUBLIC socket)


