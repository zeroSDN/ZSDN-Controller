cmake_minimum_required(VERSION 3.2)


# Specify project
set(PROJECT_NAME LinkDiscoveryModule_UT)
set(PARENT_PROJECT_NAME LinkDiscoveryModule)
project(${PROJECT_NAME})


# Include from external file
#include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/cmake_module_ut_header.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/cmake_module_ut_header_coverage.txt)

# Set source files for this project
set(SOURCE_FILES main.cpp)

# Include directories and files used to test
set(TEST_FILES
    ../LinkDiscoveryModule/LinkDiscoveryModule.cpp
    ../LinkDiscoveryModule/LinkDiscoveryModule.hpp
    )


add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${TEST_FILES} ${ADDITIONAL_DEPENDENCIES} LinkDiscoveryModuleTests.cpp LinkDiscoveryModuleTests.h)


# Link from external file
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/cmake_module_ut_link.txt)
