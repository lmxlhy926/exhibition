add_executable(spdlogTest spdlog.cpp)
target_link_libraries(spdlogTest PUBLIC spdlog)
target_link_libraries(spdlogTest PUBLIC log)