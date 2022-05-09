
set(src
    httplib.cc
    sockUtils.cpp
    socketClient.cpp
    messageHandler.cpp)

add_library(socket SHARED ${src})
target_link_libraries(socket PUBLIC qlibc)

add_subdirectory(test)