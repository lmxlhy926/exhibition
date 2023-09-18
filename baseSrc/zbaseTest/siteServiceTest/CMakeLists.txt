add_executable(site_subcribe_test siteSubscribe.cpp)
target_link_libraries(site_subcribe_test PRIVATE siteService)
target_link_libraries(site_subcribe_test PRIVATE common)
target_link_libraries(site_subcribe_test PRIVATE qlibc)
target_link_libraries(site_subcribe_test PRIVATE log)

# add_executable(site_publish_test sitePublish.cpp)
# target_link_libraries(site_publish_test PRIVATE siteService)
# target_link_libraries(site_publish_test PRIVATE log)
# target_link_libraries(site_publish_test PRIVATE common)
# target_link_libraries(site_publish_test PRIVATE qlibc)

