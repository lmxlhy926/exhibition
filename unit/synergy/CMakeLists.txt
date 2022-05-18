
set(src
    synergy.cpp
    serviceRequestHandler.cpp
    sceneCommandHandler.cpp
    util.cpp)

add_executable(synergy  ${src})
target_include_directories(synergy PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(synergy PRIVATE qlibc)
target_link_libraries(synergy PRIVATE siteService)
target_link_libraries(synergy PRIVATE socket)
target_link_libraries(synergy PRIVATE common)