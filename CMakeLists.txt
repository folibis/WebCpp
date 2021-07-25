#
# The WebCpp library
# ruslan@muhlinin.com
# July 25, 2021
#

cmake_minimum_required(VERSION 3.2)
set(CMAKE_CXX_STANDARD 11)

project(WebCpp)

file(GLOB SRC . src/*.cpp)
file(GLOB INC . include/*.h)

add_library(${PROJECT_NAME} SHARED ${SRC} ${INC})

target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include  
)

target_link_libraries(${PROJECT_NAME} PRIVATE -pthread)

add_executable(test test/main.cpp)
target_link_libraries(test PRIVATE WebCpp)
