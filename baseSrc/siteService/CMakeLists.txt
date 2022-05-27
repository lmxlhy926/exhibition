
add_library(siteService ${libType} cJSON.c service_site_manager.cpp)
target_include_directories(siteService PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
target_include_directories(siteService PUBLIC nlohmann)
target_link_libraries(siteService PUBLIC socket)

if(${TEST_ENABLE})
    add_subdirectory(test)
endif()