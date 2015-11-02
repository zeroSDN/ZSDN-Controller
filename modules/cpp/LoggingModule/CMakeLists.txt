cmake_minimum_required(VERSION 3.2)


# Specify project
set(PROJECT_NAME LoggingModule)
project(${PROJECT_NAME})


# Set ZSDN root folder path relative to project (needed for module cmake files)
set(ZSDN_DIR_RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
# Set module cmake files folder path
set(MODULE_CMAKE_FOLDER "${ZSDN_DIR_RELATIVE}/modules/cpp/cmake")


# Include from external file
include(${MODULE_CMAKE_FOLDER}/cmake_module_header.txt)

find_library(CASSANDRA_LIB libcassandra.so.2 "/usr/lib/x86_64-linux-gnu")


# Set source files for this project
set(SOURCE_FILES main.cpp)

add_executable(LoggingModule ${SOURCE_FILES} ${ADDITIONAL_DEPENDENCIES}
LoggingModule.cpp
LoggingModule.hpp
CassandraClient.cpp CassandraClient.h)


# Link from external file
include(${MODULE_CMAKE_FOLDER}/cmake_module_link.txt)
target_link_libraries(LoggingModule ${CASSANDRA_LIB})


# Custom from external file
include(${MODULE_CMAKE_FOLDER}/cmake_module_custom.txt)
