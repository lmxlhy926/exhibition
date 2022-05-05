add_library(qlibc STATIC QData.cpp)
target_link_libraries(qlibc PUBLIC jsoncpp)

add_subdirectory(test)