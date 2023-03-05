FILE(GLOB controlSrc        "control/*.cpp")
FILE(GLOB sourceManageSrc   "sourceManage/*.cpp")
FILE(GLOB serviceSrc        "service/*.cpp")
FILE(GLOB voiceControlSrc   "voiceControl/*.cpp")

set(src ${controlSrc}
        ${sourceManageSrc}
        ${serviceSrc}
        ${voiceControlSrc})

add_executable(synergy_site ${src} synergy_main.cpp)
target_link_libraries(synergy_site PRIVATE qlibc)
target_link_libraries(synergy_site PRIVATE siteService)
target_link_libraries(synergy_site PRIVATE common)
target_link_libraries(synergy_site PRIVATE log)
