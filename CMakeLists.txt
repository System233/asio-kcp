cmake_minimum_required(VERSION 3.12)
project(asio-kcp)
add_subdirectory(libs/kcp)
add_library(asio-kcp INTERFACE)
target_include_directories(asio-kcp INTERFACE libs/kcp libs/boost include)
target_link_libraries(asio-kcp INTERFACE kcp)
add_executable(test_s tests/server.cpp)
set(LIBS "")
if(WIN32)
set(LIBS ws2_32)
endif()
target_link_libraries(test_s asio-kcp ${LIBS})
add_executable(test_c tests/client.cpp)
target_link_libraries(test_c asio-kcp ${LIBS})
