cmake_minimum_required(VERSION 3.10)

file(GLOB TEST_SOURCES "*.asm" "*.cpp" "*.c")
file(GLOB TEST_HEADERS "*.inc" "*.h" "*.hpp")

set(TEST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(TEST_SOURCES ${TEST_SOURCES} ${TEST_HEADERS} PARENT_SCOPE)
