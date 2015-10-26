cmake_minimum_required(VERSION 3.2)


# Specify project
set(PROJECT_NAME StatisticsModule)
project(${PROJECT_NAME})


# Include from external file
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/cmake_module_header.txt)


# Set source files for this project
set(SOURCE_FILES main.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES}
    ${ADDITIONAL_DEPENDENCIES}
    StatisticsModule.cpp StatisticsModule.hpp
    )


# Link from external file
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/cmake_module_link.txt)


# Custom from external file
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/cmake_module_custom.txt)
