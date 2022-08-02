FILE(GLOB jsoncppSrc "jsoncpp/*.cpp")
FILE(GLOB qlibcSrc "*.cpp")

set(src ${jsoncppSrc} ${qlibcSrc})

add_library(qlibc STATIC ${src})
target_include_directories(qlibc PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)

add_subdirectory(test)
