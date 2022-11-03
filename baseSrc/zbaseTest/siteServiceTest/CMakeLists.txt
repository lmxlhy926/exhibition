#add_executable(site_send_test send.cpp)
#target_link_libraries(site_send_test PRIVATE siteService)
#target_link_libraries(site_send_test PRIVATE log)
#target_link_libraries(site_send_test PRIVATE common)
#target_link_libraries(site_send_test PRIVATE qlibc)
#
#add_executable(site_receive_test receive.cpp)
#target_link_libraries(site_receive_test PRIVATE siteService)
#target_link_libraries(site_receive_test PRIVATE log)
#target_link_libraries(site_receive_test PRIVATE common)
#target_link_libraries(site_receive_test PRIVATE qlibc)

add_executable(site_subcribe_test siteSubscribe.cpp)
target_link_libraries(site_subcribe_test PRIVATE siteService)
target_link_libraries(site_subcribe_test PRIVATE log)
target_link_libraries(site_subcribe_test PRIVATE common)
target_link_libraries(site_subcribe_test PRIVATE qlibc)

#add_executable(site_publish_test sitePublish.cpp)
#target_link_libraries(site_publish_test PRIVATE siteService)
#target_link_libraries(site_publish_test PRIVATE log)
#target_link_libraries(site_publish_test PRIVATE common)
#target_link_libraries(site_publish_test PRIVATE qlibc)

