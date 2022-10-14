add_executable(siteServeice_test service_provider_example.cpp)
target_link_libraries(siteServeice_test PRIVATE siteService)


add_executable(site_subcribe_test siteSubscribe.cpp)
target_link_libraries(site_subcribe_test PRIVATE siteService)
target_link_libraries(site_subcribe_test PRIVATE log)

add_executable(site_publish_test sitePublish.cpp)
target_link_libraries(site_publish_test PRIVATE siteService)
target_link_libraries(site_publish_test PRIVATE log)


add_executable(send send.cpp)
target_link_libraries(send PRIVATE siteService)
target_link_libraries(send PRIVATE log)
target_link_libraries(send PRIVATE common)
target_link_libraries(send PRIVATE qlibc)


add_executable(receive receive.cpp)
target_link_libraries(receive PRIVATE siteService)
target_link_libraries(receive PRIVATE log)
target_link_libraries(receive PRIVATE common)
target_link_libraries(receive PRIVATE qlibc)



