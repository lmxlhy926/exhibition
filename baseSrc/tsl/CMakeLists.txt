add_library(tsl INTERFACE)
target_include_directories(tsl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tsl INTERFACE ${CMAKE_SOURCE_DIR}/baseSrc/tsl/libcrypto.a)
target_link_libraries(tsl INTERFACE ${CMAKE_SOURCE_DIR}/baseSrc/tsl/libssl.a)