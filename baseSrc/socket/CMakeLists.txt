#set(src test.cpp httplib.cc sockUtils.cpp)
#add_library(socket STATIC ${src})
#target_include_directories(socket PRIVATE ${CMAKE_SOURCE_DIR}/baseSrc)


set(src test.cpp httplib.cc sockUtils.cpp socketClient.cpp messageHandler.cpp objectHolder.cpp)
add_executable(socket  ${src})
target_include_directories(socket PRIVATE ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(socket PRIVATE qlibc)

add_subdirectory(test)