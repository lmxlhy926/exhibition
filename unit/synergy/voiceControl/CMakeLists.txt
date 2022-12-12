add_library(voice voiceStringMatch.cpp)
target_include_directories(voice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(voice PRIVATE qlibc)
target_link_libraries(voice PRIVATE common)