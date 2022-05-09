add_library(jsoncpp SHARED json_reader.cpp json_value.cpp json_writer.cpp)
target_include_directories(jsoncpp PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)
