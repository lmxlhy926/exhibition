FILE(GLOB src "*.cpp")
add_library(bleLogic STATIC ${src})
target_include_directories(bleLogic INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(bleLogic PUBLIC qlibc)
target_link_libraries(bleLogic PUBLIC log)
target_link_libraries(bleLogic PUBLIC format)
