cmake_minimum_required(VERSION 3.10)

project(WebSocketServer)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenSSL REQUIRED)

aux_source_directory(. DIR_CURR)

add_executable(WebSocketServer ${DIR_CURR})

target_link_libraries(WebSocketServer OpenSSL::SSL OpenSSL::Crypto)