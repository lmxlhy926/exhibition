
FILE(GLOB sourceSrc     "source/*.cpp")
FILE(GLOB serviceSrc    "service/*.cpp")
FILE(GLOB nightLightSrc "*.cpp")
set(src ${sourceSrc}
        ${serviceSrc}
        ${nightLightSrc}       
)

add_executable(light_flow main.cpp ${src})
target_include_directories(light_flow PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(light_flow PRIVATE qlibc)
target_link_libraries(light_flow PRIVATE log)
target_link_libraries(light_flow PRIVATE common)
target_link_libraries(light_flow PRIVATE siteService)
