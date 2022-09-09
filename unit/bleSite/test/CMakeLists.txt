#add_executable(serialTest serialTest.cpp)
#target_link_libraries(serialTest PRIVATE qlibc)
#target_link_libraries(serialTest PRIVATE format)
#target_link_libraries(serialTest PRIVATE serial)

add_executable(httpRequest httpRequest.cpp)
target_link_libraries(httpRequest PRIVATE qlibc)
target_link_libraries(httpRequest PRIVATE format)
target_link_libraries(httpRequest PRIVATE serial)
target_link_libraries(httpRequest PRIVATE socket)
target_link_libraries(httpRequest PRIVATE common)

add_executable(bleSiteRequest bleRequest.cpp)
target_link_libraries(bleSiteRequest PRIVATE qlibc)
target_link_libraries(bleSiteRequest PRIVATE format)
target_link_libraries(bleSiteRequest PRIVATE serial)
target_link_libraries(bleSiteRequest PRIVATE socket)



add_executable(logtest logtest.cpp)
target_link_libraries(logtest PRIVATE qlibc)
target_link_libraries(logtest PRIVATE format)
target_link_libraries(logtest PRIVATE log)
target_link_libraries(logtest PRIVATE common)

add_executable(snAddrTest snAddrTest.cpp)
target_link_libraries(snAddrTest PRIVATE qlibc)
target_link_libraries(snAddrTest PRIVATE format)
target_link_libraries(snAddrTest PRIVATE log)
target_link_libraries(snAddrTest PRIVATE common)
target_link_libraries(snAddrTest PRIVATE bleLogic)



