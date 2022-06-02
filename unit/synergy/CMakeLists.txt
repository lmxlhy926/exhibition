FILE(GLOB mainSrc "*.cpp")
FILE(GLOB subSrc "deviceControl/*.cpp")
set(src ${mainSrc} ${subSrc})

add_executable(synergy ${src})
target_link_libraries(synergy PRIVATE qlibc)
target_link_libraries(synergy PRIVATE siteService)
target_link_libraries(synergy PRIVATE socket)
target_link_libraries(synergy PRIVATE common)