
set(src
    httplib.cc
    sockUtils.cpp
    socketClient.cpp
    messageHandler.cpp
    socketServer.cpp)

add_library(socket STATIC ${src})
target_link_libraries(socket PUBLIC qlibc)

add_subdirectory(test)