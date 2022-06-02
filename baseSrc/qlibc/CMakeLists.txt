FILE(GLOB jsoncppSrc "jsoncpp/*.cpp")
FILE(GLOB qlibcSrc "*.cpp")

set(src ${jsoncppSrc} ${qlibcSrc})

set(src ${jsoncppSrc} FileUtils.cpp QData.cpp StringUtils.cpp)

add_library(qlibc ${libType} ${src})
target_include_directories(qlibc PUBLIC ${CMAKE_SOURCE_DIR}/baseSrc)

if(${TEST_ENABLE})
#    add_subdirectory(test)
endif()