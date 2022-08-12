FILE(GLOB mainSrc "*.cpp")
FILE(GLOB subSrc "deviceControl/*.cpp")
set(src ${mainSrc} ${subSrc})

add_executable(synergy_site ${src})
target_link_libraries(synergy_site PRIVATE qlibc)
target_link_libraries(synergy_site PRIVATE siteService)
target_link_libraries(synergy_site PRIVATE common)
target_link_libraries(synergy_site PRIVATE log)