
FILE(GLOB sourceSrc     "source/*.cpp")
FILE(GLOB serviceSrc    "service/*.cpp")
FILE(GLOB nightLightSrc "*.cpp")
set(src ${sourceSrc}
        ${serviceSrc}
        ${nightLightSrc}       
)

add_executable(nightLight_site main.cpp ${src})
target_include_directories(nightLight_site PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(nightLight_site PRIVATE qlibc)
target_link_libraries(nightLight_site PRIVATE log)
target_link_libraries(nightLight_site PRIVATE common)
target_link_libraries(nightLight_site PRIVATE siteService)
