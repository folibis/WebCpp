# The WebCpp library
# ruslan@muhlinin.com
# July 25, 2021


option(ZLIB "Add zlib support" OFF)
option(OPENSSL "Add openssl/https support" ON)

set(ZLIB_URL https://zlib.net/)
set(ZLIB_VERSION zlib-1.2.11)
set(ZLIB_ARCHIVE zlib-1.2.11.tar.gz)
set(ZLIB_HASH_METHOD SHA256)
set(ZLIB_HASH c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1)


cmake_minimum_required(VERSION 3.11)
set(CMAKE_CXX_STANDARD 11)

project(webcpp)

file(GLOB SRC . src/*.cpp)
file(GLOB INC . include/*.h)

add_library(${PROJECT_NAME} SHARED ${SRC} ${INC})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
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

add_executable(test test/main.cpp)
target_link_libraries(test PRIVATE ${PROJECT_NAME})
