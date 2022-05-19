add_library(aes STATIC aes.c)
target_include_directories(aes PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)

add_library(aes_shared SHARED aes.c)
target_include_directories(aes_shared PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
