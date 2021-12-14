# The WebCpp library
# ruslan@muhlinin.com
# July 25, 2021

cmake_minimum_required(VERSION 3.11)
set(CMAKE_CXX_STANDARD 11)

project(webcpp-examples)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

add_definitions(-DPUBLIC_DIR="${CMAKE_CURRENT_SOURCE_DIR}/public")

add_executable(SimpleHttpServer SimpleHttpServer.cpp)
target_link_libraries(SimpleHttpServer PRIVATE webcpp)

add_executable(HttpServer HttpServer.cpp)
target_link_libraries(HttpServer PRIVATE webcpp)

add_executable(FileServer FileServer.cpp)
target_link_libraries(FileServer PRIVATE webcpp)

add_executable(Form Form.cpp)
target_link_libraries(Form PRIVATE webcpp)

add_executable(Route Route.cpp)
target_link_libraries(Route PRIVATE webcpp)

add_executable(PreRoute PreRoute.cpp)
target_link_libraries(PreRoute PRIVATE webcpp)

if(WEBSOCKET)
    add_executable(WebSocketServer WebSocketServer.cpp)
    target_link_libraries(WebSocketServer PRIVATE webcpp)

    add_executable(WebSocketPingPong WebSocketPingPong.cpp)
    target_link_libraries(WebSocketPingPong PRIVATE webcpp)

    add_executable(WebSocketClient WebSocketClient.cpp)
    target_link_libraries(WebSocketClient PRIVATE webcpp)
endif()

if(FASTCGI)
    add_executable(FastCgi FastCgi.cpp)
    target_link_libraries(FastCgi PRIVATE webcpp)
endif()

add_executable(HttpClient HttpClient.cpp)
target_link_libraries(HttpClient PRIVATE webcpp)
