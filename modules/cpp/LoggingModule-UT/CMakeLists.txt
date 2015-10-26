cmake_minimum_required(VERSION 3.2)


# Specify project
set(PROJECT_NAME LoggingModule_UT)
set(PARENT_PROJECT_NAME LoggingModule)
project(${PROJECT_NAME})


# Include from external file
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/cmake_module_ut_header.txt)


# Set source files for this project
set(SOURCE_FILES main.cpp)

# Include directories and files used to test
set(TEST_FILES
    )


add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${TEST_FILES} ${ADDITIONAL_DEPENDENCIES} LoggingModuleTests.cpp LoggingModuleTests.h)


# Link from external file
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/cmake_module_ut_link.txt)
