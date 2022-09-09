set(SOURCE
       posixSerial.cpp
        telinkDongle.cpp)
add_library(serial ${SOURCE})
target_include_directories(serial PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(serial PUBLIC common)

add_subdirectory(test)