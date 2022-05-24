
set(src
    httplib.cc
    sockUtils.cpp
    socketClient.cpp
    messageHandler.cpp
#    socketServer.cpp)

add_library(socket ${libType} ${src})
target_include_directories(socket PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(socket PUBLIC qlibc)

#add_subdirectory(test)