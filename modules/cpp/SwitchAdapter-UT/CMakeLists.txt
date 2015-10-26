cmake_minimum_required(VERSION 3.2)


# Specify project
set(PROJECT_NAME SwitchAdapter_UT)
set(PARENT_PROJECT_NAME SwitchAdapter)
project(${PROJECT_NAME})


# Include from external file
#include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/cmake_module_ut_header.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/cmake_module_ut_header_coverage.txt)

# Set source files for this project
set(SOURCE_FILES main.cpp)

# Include directories and files used to test
set(TEST_FILES
../SwitchAdapter/SwitchAdapter.hpp
../SwitchAdapter/SwitchAdapter.cpp
../SwitchAdapter/SwitchAdapterRunner.h
../SwitchAdapter/SwitchAdapterRunner.cpp
../SwitchAdapter/SwitchConnectionUtil.h
../SwitchAdapter/SwitchConnectionUtil.cpp
)

add_executable(SwitchAdapter_UT ${SOURCE_FILES} ${TEST_FILES} ${ADDITIONAL_DEPENDENCIES} SwitchAdapterTests.cpp SwitchAdapterTests.h)

# Link from external file
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/cmake_module_ut_link.txt)