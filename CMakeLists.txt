# The WebCpp library
# ruslan@muhlinin.com
# July 25, 2021


option(ZLIB "Add zlib support" OFF)
option(OPENSSL "Add openssl/https support" OFF)
option(WEBSOCKET "Add websocket support" ON)
option(FASTCGI "Add FastCGI support" ON)
option(EXAMPLES "Build examples" ON)

set(ZLIB_URL https://zlib.net/)
set(ZLIB_VERSION zlib-1.2.11)
set(ZLIB_ARCHIVE zlib-1.2.11.tar.gz)
set(ZLIB_HASH_METHOD SHA256)
set(ZLIB_HASH c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1)

cmake_minimum_required(VERSION 3.11)
set(CMAKE_CXX_STANDARD 11)

project(webcpp)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

file(GLOB SRC . src/*.cpp src/conn/*.cpp src/utils/*.cpp)
file(GLOB INC . include/*.h include/common/*.h include/conn/*.h include/utils/*.h)

add_library(${PROJECT_NAME} SHARED ${SRC} ${INC})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/common
    ${CMAKE_CURRENT_SOURCE_DIR}/include/conn
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utils
    )

target_link_libraries(${PROJECT_NAME} PRIVATE -pthread)

if(ZLIB)
    add_definitions(-DWITH_ZLIB)
    if(NOT EXISTS ${CMAKE_BINARY_DIR}/${ZLIB_ARCHIVE})
        message(STATUS "Downloading zlib archive")
        file(DOWNLOAD ${ZLIB_URL}${ZLIB_ARCHIVE}
            ${CMAKE_BINARY_DIR}/${ZLIB_ARCHIVE}
            TIMEOUT 10
            EXPECTED_HASH ${ZLIB_HASH_METHOD}=${ZLIB_HASH}
            SHOW_PROGRESS)
    endif()

    if(NOT ${CMAKE_BINARY_DIR}/${ZLIB_ARCHIVE})
    message(STATUS "Unzipping the zlib archive")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_BINARY_DIR}/${ZLIB_ARCHIVE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external
        )
    endif()

    message(STATUS "Configure with zlib support")
    add_subdirectory(external/${ZLIB_VERSION})
    target_link_libraries(${PROJECT_NAME} PRIVATE zlib)
endif()

if(OPENSSL)
    message(STATUS "Configure with OpenSSL support")
    add_definitions(-DWITH_OPENSSL)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL)
endif()

if(WEBSOCKET)
    message(STATUS "Configure with WebSocket support")
    add_definitions(-DWITH_WEBSOCKET)
endif()

if(FASTCGI)
    message(STATUS "Configure with FastCGI support")
    add_definitions(-DWITH_FASTCGI)
endif()

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

add_definitions(-DPUBLIC_DIR="${CMAKE_CURRENT_SOURCE_DIR}/examples/public")

if(EXAMPLES)
    add_executable(Simple examples/Simple.cpp)
    target_link_libraries(Simple PRIVATE ${PROJECT_NAME})

    add_executable(FileServer examples/FileServer.cpp)
    target_link_libraries(FileServer PRIVATE ${PROJECT_NAME})

    add_executable(Form examples/Form.cpp)
    target_link_libraries(Form PRIVATE ${PROJECT_NAME})

    add_executable(Route examples/Route.cpp)
    target_link_libraries(Route PRIVATE ${PROJECT_NAME})

    add_executable(PreRoute examples/PreRoute.cpp)
    target_link_libraries(PreRoute PRIVATE ${PROJECT_NAME})

    if(WEBSOCKET)
        add_executable(WebSocket examples/WebSocket.cpp)
        target_link_libraries(WebSocket PRIVATE ${PROJECT_NAME})
    endif()

    if(FASTCGI)
        add_executable(FastCgi examples/FastCgi.cpp)
        target_link_libraries(FastCgi PRIVATE ${PROJECT_NAME})
    endif()
endif()
