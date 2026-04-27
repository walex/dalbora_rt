cmake_minimum_required(VERSION 3.10)

file(GLOB OS_SOURCES "*.asm" "*.cpp" "*.c")
file(GLOB OS_HEADERS "*.inc" "*.h" "*.hpp")

set(OS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(OS_SOURCES ${OS_SOURCES} PARENT_SCOPE)
set(OS_HEADERS ${OS_HEADERS} PARENT_SCOPE)