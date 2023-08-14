
FILE(GLOB sourceSrc "source/*.cpp")
FILE(GLOB nightLightSrc "*.cpp")
set(src ${sourceSrc}
        ${nightLightSrc}
)

add_executable(nightLight_site main.cpp ${src})
target_link_libraries(nightLight_site PRIVATE qlibc)
target_link_libraries(nightLight_site PRIVATE log)
target_link_libraries(nightLight_site PRIVATE common)
target_link_libraries(nightLight_site PRIVATE siteService)