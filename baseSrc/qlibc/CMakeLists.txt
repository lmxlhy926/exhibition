
set(src
        jsoncpp/json_reader.cpp
        jsoncpp/json_value.cpp
        jsoncpp/json_writer.cpp
        QData.cpp
        StringUtils.cpp
        FileUtils.cpp)

add_library(qlibc STATIC ${src})
target_include_directories(qlibc PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)


add_library(qlibc_shared SHARED ${src})
target_include_directories(qlibc_shared PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)



add_subdirectory(test)