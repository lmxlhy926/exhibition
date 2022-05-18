add_library(common STATIC configParamUtil.cpp httpUtil.cpp)
target_link_libraries(common PUBLIC qlibc)

