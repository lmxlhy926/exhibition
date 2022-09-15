FILE(GLOB src "*.cpp")
add_library(commonSerial STATIC ${src})
target_include_directories(commonSerial INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
#target_link_libraries(bleLogic PUBLIC qlibc)
#target_link_libraries(bleLogic PUBLIC log)
#target_link_libraries(bleLogic PUBLIC format)
#target_link_libraries(bleLogic PUBLIC common)

