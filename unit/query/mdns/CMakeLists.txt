aux_source_directory(. MDNS_SOURCE_FILES)
add_library(mdns STATIC ${MDNS_SOURCE_FILES})
target_include_directories(mdns PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(mdns PUBLIC sitemg)
