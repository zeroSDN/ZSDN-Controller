cmake_minimum_required(VERSION 3.2)


# Specify project
set(PROJECT_NAME SwitchRegistryModule)
project(${PROJECT_NAME})


# Set ZSDN root folder path relative to project (needed for module cmake files)
set(ZSDN_DIR_RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
# Set module cmake files folder path
set(MODULE_CMAKE_FOLDER "${ZSDN_DIR_RELATIVE}/modules/cpp/cmake")


# Include from external file
include(${MODULE_CMAKE_FOLDER}/cmake_module_header.txt)


# Set source files for this project
set(SOURCE_FILES main.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES}
    ${ADDITIONAL_DEPENDENCIES}
    SwitchRegistryModule.cpp SwitchRegistryModule.hpp
    Switch.h Switch.cpp Port.h Port.cpp)


# Link from external file
include(${MODULE_CMAKE_FOLDER}/cmake_module_link.txt)


# Custom from external file
include(${MODULE_CMAKE_FOLDER}/cmake_module_custom.txt)
