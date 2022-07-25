
set(SOURCE
        BLETelinkDongle.cpp
        CommonDongle.cpp
        BaseSerialPort.cpp
        PosixSerialPort.cpp)
add_library(serial ${SOURCE})
target_include_directories(serial PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
