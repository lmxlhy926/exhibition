
add_executable(configTest mqttpayloadtest.cpp)
target_include_directories(configTest PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(configTest PRIVATE qlibc)
