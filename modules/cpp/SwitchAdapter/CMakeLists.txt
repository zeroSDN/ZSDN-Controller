cmake_minimum_required(VERSION 3.2)


# Specify project
set(PROJECT_NAME SwitchAdapter)
project(${PROJECT_NAME})


# Set ZSDN root folder path relative to project (needed for module cmake files)
set(ZSDN_DIR_RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
# Set module cmake files folder path
set(MODULE_CMAKE_FOLDER "${ZSDN_DIR_RELATIVE}/modules/cpp/cmake")


# Include from external file
include(${MODULE_CMAKE_FOLDER}/cmake_module_header.txt)

# Set source files for this project
set(SOURCE_FILES main.cpp)

add_executable(SwitchAdapter ${SOURCE_FILES}
${ADDITIONAL_DEPENDENCIES}
SwitchAdapter.hpp SwitchAdapter.cpp
SwitchAdapterRunner.h SwitchAdapterRunner.cpp
SwitchConnectionUtil.h SwitchConnectionUtil.cpp
)


# Link from external file
include(${MODULE_CMAKE_FOLDER}/cmake_module_link.txt)


# Optional: Specify UT params
set(UT_PARAMS "${CMAKE_CURRENT_SOURCE_DIR}/../${PROJECT_NAME}-UT/module-ut.config")


# Custom from external file
include(${MODULE_CMAKE_FOLDER}/cmake_module_custom.txt)
