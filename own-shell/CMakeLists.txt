cmake_minimum_required(VERSION 3.10)
project(own_shell C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB SRC_FILES "src/*.c")
list(APPEND SRC_FILES "main.c")

add_executable(shell ${SRC_FILES})

target_include_directories(shell PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_compile_options(shell PRIVATE -Wall -Wextra -Wpedantic)

# DEBUG and RELEASE options
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(shell PRIVATE -g -O0)
  message(STATUS "Building in DEBUG mode")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_options(shell PRIVATE -O3 -DNDEBUG)
  message(STATUS "Building in RELEASE mode")
else()
  message(STATUS "Building in DEFAULT mode")
endif()

target_link_libraries(shell PRIVATE)