add_executable(config myutil.cpp baseFunction.cpp config.cpp)
target_include_directories(config PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(config PRIVATE encrypt)
target_link_libraries(config PRIVATE qlibc)