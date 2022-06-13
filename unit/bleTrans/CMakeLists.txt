
FILE(GLOB mainSrc "*.cpp")
FILE(GLOB subSrc "bleControl/*.cpp")
set(src ${mainSrc} ${subSrc})

add_executable(bleTrans ${src})
target_link_libraries(bleTrans PRIVATE qlibc)
target_link_libraries(bleTrans PRIVATE siteService)
target_link_libraries(bleTrans PRIVATE socket)
target_link_libraries(bleTrans PRIVATE common)

add_subdirectory(test)