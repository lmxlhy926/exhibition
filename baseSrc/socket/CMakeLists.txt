
set(src
    httplib.cc
    sockUtils.cpp
    socketClient.cpp
    messageHandler.cpp
    socketServer.cpp)

add_library(socket STATIC ${src})
target_link_libraries(socket PUBLIC qlibc)

add_library(socket_shared SHARED ${src})
target_link_libraries(socket_shared PUBLIC qlibc_shared)

add_subdirectory(test)