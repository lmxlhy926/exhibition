add_library(mdnsBasic STATIC mdns.c mdnsUtil.c)
target_include_directories(mdnsBasic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)