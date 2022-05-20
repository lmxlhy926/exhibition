
set(src
    httplib.cc
    sockUtils.cpp
    socketClient.cpp
    messageHandler.cpp)

add_library(socket STATIC ${src})
target_include_directories(socket PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(socket PUBLIC qlibc)

add_library(socket_shared SHARED ${src})
target_link_libraries(socket_shared PUBLIC qlibc_shared)
target_include_directories(socket_shared PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)


add_subdirectory(test)