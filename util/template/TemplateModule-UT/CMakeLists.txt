cmake_minimum_required(VERSION 3.2)


# Specify project
set(PROJECT_NAME TemplateModule_UT)
set(PARENT_PROJECT_NAME TemplateModule)
project(${PROJECT_NAME})


# Set ZSDN root folder path relative to project (needed for module cmake files)
set(ZSDN_DIR_RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
# Set module cmake files folder path
set(MODULE_CMAKE_FOLDER "${ZSDN_DIR_RELATIVE}/modules/cpp/cmake")


# Include from external file
include(${MODULE_CMAKE_FOLDER}/cmake_module_ut_header.txt)
#include(${MODULE_CMAKE_FOLDER}/cmake_module_ut_header_coverage.txt)

# Set source files for this project
set(SOURCE_FILES main.cpp)

# Include directories and files used to test
set(TEST_FILES
    ../TemplateModule/TemplateModule.hpp
    ../TemplateModule/TemplateModule.cpp
    )


add_executable(TemplateModule_UT ${SOURCE_FILES} ${TEST_FILES} ${ADDITIONAL_DEPENDENCIES} TemplateModuleTests.cpp TemplateModuleTests.h )



# Link from external file
include(${MODULE_CMAKE_FOLDER}/cmake_module_ut_link.txt)