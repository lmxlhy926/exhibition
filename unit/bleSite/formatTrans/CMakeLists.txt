FILE(GLOB src "*.cpp")
add_library(format STATIC ${src})
target_link_libraries(format PRIVATE qlibc)
target_include_directories(format INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})