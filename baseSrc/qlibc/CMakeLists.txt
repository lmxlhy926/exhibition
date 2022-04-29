add_library(qlibc STATIC StringUtils.cpp QData.cpp)
target_link_libraries(qlibc PUBLIC jsoncpp)