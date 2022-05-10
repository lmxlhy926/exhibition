add_executable(siteServeice_test service_provider_example.cpp)
target_link_libraries(siteServeice_test PRIVATE siteService)
target_include_directories(siteServeice_test PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/siteService)


add_executable(site_subcribe siteSubscribe.cpp)
target_link_libraries(site_subcribe PRIVATE siteService)
target_include_directories(site_subcribe PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/siteService)



add_executable(site_publish sitePublish.cpp)
target_link_libraries(site_publish PRIVATE siteService)
target_include_directories(site_publish PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/siteService)