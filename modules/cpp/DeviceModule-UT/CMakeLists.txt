cmake_minimum_required(VERSION 3.2)


# Specify project
set(PROJECT_NAME DeviceModule_UT)
set(PARENT_PROJECT_NAME DeviceModule)
project(${PROJECT_NAME})


# Include from external file
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/cmake_module_ut_header.txt)
#include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/cmake_module_ut_header_coverage.txt)


# Set source files for this project
set(SOURCE_FILES main.cpp)

# Include directories and files used to test
set(TEST_FILES
    ../DeviceModule/DeviceModule.hpp
    ../DeviceModule/DeviceModule.cpp
    ../DeviceModule/Device.h
    ../DeviceModule/Device.cpp
    )

add_executable(DeviceModule_UT ${SOURCE_FILES} ${TEST_FILES} ${ADDITIONAL_DEPENDENCIES} DeviceModuleTests.cpp DeviceModuleTests.h )

# Link from external file
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/cmake_module_ut_link.txt)