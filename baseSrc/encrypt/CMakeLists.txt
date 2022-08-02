FILE(GLOB src "src/*.c")

add_library(encrypt STATIC ${src})
target_include_directories(encrypt PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/)
target_include_directories(encrypt PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/encrypt/include)
target_include_directories(encrypt PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/encrypt/include/mbedtls)
target_include_directories(encrypt PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/encrypt/include/secp256k1_bitcoin)
target_include_directories(encrypt PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/encrypt/temp)
target_include_directories(encrypt PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc/encrypt/src/secp256k1_bitcoin)


