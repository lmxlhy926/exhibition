FILE(GLOB src "*.cpp")

add_executable(mdm_site  ${src} mdm_main.cpp mdmParam.hpp handle/mdmConfig.cpp handle/mdmConfig.h handle/mqttHandle.cpp handle/mqttHandle.cpp handle/mqttHandle.h sslUtil/ssLUtil.cpp sslUtil/ssLUtil.h)
target_link_libraries(mdm_site PRIVATE qlibc)
target_link_libraries(mdm_site PRIVATE log)
target_link_libraries(mdm_site PRIVATE mqttclient)
target_link_libraries(mdm_site PRIVATE configUtil)