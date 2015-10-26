cmake_minimum_required(VERSION 3.2)


# Specify project
set(PROJECT_NAME SimpleForwardingModule_UT)
set(PARENT_PROJECT_NAME SimpleForwardingModule)
project(${PROJECT_NAME})


# Include from external file
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/cmake_module_ut_header.txt)


# Set source files for this project
set(SOURCE_FILES main.cpp)

# Include directories and files used to test
set(TEST_FILES
    ../SimpleForwardingModule/SimpleForwardingModule.cpp
    ../SimpleForwardingModule/SimpleForwardingModule.hpp
    )


add_executable(SimpleForwardingModule_UT ${SOURCE_FILES} ${TEST_FILES} ${ADDITIONAL_DEPENDENCIES} SimpleForwardingModuleTests.cpp SimpleForwardingModuleTests.h)


# Link from external file
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/cmake_module_ut_link.txt)