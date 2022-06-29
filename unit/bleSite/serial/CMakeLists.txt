
IF(CYGWIN)
    message("CYGWIN")
    add_library(serial BaseSerialPort.cpp  Win32SerialPort.cpp BLETelinkDongle.cpp CommonDongle.cpp)
    target_include_directories(serial PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
ELSE()
    add_library(serial BaseSerialPort.cpp  PosixSerialPort.cpp BLETelinkDongle.cpp CommonDongle.cpp )
ENDIF()