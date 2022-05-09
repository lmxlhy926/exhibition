#set(src test.cpp httplib.cc sockUtils.cpp)
#add_library(socket STATIC ${src})
#target_include_directories(socket PRIVATE ${CMAKE_SOURCE_DIR}/baseSrc)


set(src
    httplib.cc
    sockUtils.cpp
    socketClient.cpp
    messageHandler.cpp
    objecPtrtHolder.cpp
    socketServer.cpp)

add_library(socket SHARED ${src})
target_link_libraries(socket PUBLIC qlibc)

add_subdirectory(test)