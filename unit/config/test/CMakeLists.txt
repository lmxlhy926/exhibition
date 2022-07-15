
#add_executable(mqttpayloadtest mqttpayloadtest.cpp)
#target_link_libraries(mqttpayloadtest PRIVATE configUtil)
#
#
#add_executable(configTest test.cpp)
#target_include_directories(configTest PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
#target_link_libraries(configTest PRIVATE qlibc)
#target_link_libraries(configTest PRIVATE common)


add_executable(whiteListScribe whiteListPulish.cpp)
target_link_libraries(whiteListScribe PRIVATE qlibc)
target_link_libraries(whiteListScribe PRIVATE common)
target_link_libraries(whiteListScribe PRIVATE log)
target_link_libraries(whiteListScribe PUBLIC siteService)

