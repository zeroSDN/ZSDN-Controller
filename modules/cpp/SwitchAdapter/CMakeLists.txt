cmake_minimum_required(VERSION 3.2)


# Specify project
set(PROJECT_NAME SwitchAdapter)
project(${PROJECT_NAME})


# Set ZSDN root folder path relative to project
set(ZSDN_ROOT_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/../../..")


# Include from external file
include(${ZSDN_ROOT_FOLDER}/cmake/cmake_module_header.txt)

# Set source files for this project
set(SOURCE_FILES main.cpp)

add_executable(SwitchAdapter ${SOURCE_FILES}
${ADDITIONAL_DEPENDENCIES}
SwitchAdapter.hpp SwitchAdapter.cpp
SwitchAdapterRunner.h SwitchAdapterRunner.cpp
SwitchConnectionUtil.h SwitchConnectionUtil.cpp
)


# Link from external file
include(${ZSDN_ROOT_FOLDER}/cmake/cmake_module_link.txt)


# Optional: Specify UT params
set(UT_PARAMS "${CMAKE_CURRENT_SOURCE_DIR}/../${PROJECT_NAME}-UT/module-ut.config")


# Custom from external file
include(${ZSDN_ROOT_FOLDER}/cmake/cmake_module_custom.txt)
