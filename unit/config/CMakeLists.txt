add_library(config STATIC myutil.cpp baseFunction.cpp)
target_include_directories(config PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
target_link_libraries(config PUBLIC encrypt)