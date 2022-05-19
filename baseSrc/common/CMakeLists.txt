add_library(common STATIC configParamUtil.cpp httpUtil.cpp)
target_link_libraries(common PUBLIC qlibc)

add_library(common_shared SHARED configParamUtil.cpp httpUtil.cpp)
target_link_libraries(common_shared PUBLIC qlibc_shared)
target_link_libraries(common_shared PUBLIC socket_shared)


