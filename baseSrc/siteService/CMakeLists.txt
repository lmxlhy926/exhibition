add_library(siteService STATIC cJSON.c httplib.cpp service_site_manager.cpp)
target_include_directories(siteService PUBLIC nlohmann)

add_subdirectory(test)