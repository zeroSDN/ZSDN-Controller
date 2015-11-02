cmake_minimum_required(VERSION 3.2)


# Specify project
set(PROJECT_NAME SwitchRegistryModule_UT)
set(PARENT_PROJECT_NAME SwitchRegistryModule)
project(${PROJECT_NAME})


# Set ZSDN root folder path relative to project (needed for module cmake files)
set(ZSDN_DIR_RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
# Set module cmake files folder path
set(MODULE_CMAKE_FOLDER "${ZSDN_DIR_RELATIVE}/modules/cpp/cmake")


# Include from external file
#include(${MODULE_CMAKE_FOLDER}/cmake_module_ut_header.txt)
include(${MODULE_CMAKE_FOLDER}/cmake_module_ut_header_coverage.txt)

# Set source files for this project
set(SOURCE_FILES main.cpp)

# Include directories and files used to test
set(TEST_FILES
    ../SwitchRegistryModule/SwitchRegistryModule.cpp
    ../SwitchRegistryModule/SwitchRegistryModule.hpp
    ../SwitchRegistryModule/Switch.cpp
    ../SwitchRegistryModule/Switch.h
    ../SwitchRegistryModule/Port.cpp
    ../SwitchRegistryModule/Port.h
    )


add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${TEST_FILES} ${ADDITIONAL_DEPENDENCIES} SwitchRegistryModuleTests.cpp SwitchRegistryModuleTests.h)


# Link from external file
include(${MODULE_CMAKE_FOLDER}/cmake_module_ut_link.txt)
