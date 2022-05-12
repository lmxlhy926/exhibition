
set(src
    config.cpp
    myutil.cpp
    cloudUtil.cpp
    serviceRequestHandler.cpp
    subscribeMessageHandler.cpp
    configParamUtil.cpp)

add_executable(config  ${src})
target_include_directories(config PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(config PRIVATE encrypt)
target_link_libraries(config PRIVATE qlibc)
target_link_libraries(config PRIVATE siteService)
target_link_libraries(config PRIVATE socket)
target_link_libraries(config PRIVATE aes)


#add_executable(logicTest logic.cpp myutil.cpp cloudUtil.cpp)
#target_link_libraries(logicTest PRIVATE encrypt)
#target_link_libraries(logicTest PRIVATE qlibc)
#
#add_subdirectory(test)