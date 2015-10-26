#!/usr/bin/env bash
# @author Tobias Freundorfer
# @date 10.08.2015
# @version 1.0

# SET Variables BELOW HERE #
# ---------------------------------------------------------------------------------------- #

# Binary Name (Is needed to run the Project and therefore generate the dynamic .gcda files.)
binaryFilename=DeviceModule_UT # ENTER YOUR BINARY FILENAME HERE !! (no spaces between '=')

# The directoy where the Output Html Files should be generated to.
genhtmlOutputDir=/home/zsdn/Desktop/CodeCoverageHtml

# ---------------------------------------------------------------------------------------- #


# DO NOT EDIT BELOW HERE #

# The directory where the CMake Files and therefore also the gcov files are generated to.
cmakeGeneratedDir=./CMakeFiles/$binaryFilename.dir/

# ---------------------------------------------------------------------------------------- #
echo "--> Cleaning old generated data of the given Project ..."
rm -rf $cmakeGeneratedDir
rm -rf $genhtmlOutputDir
rm ./CMakeCache.txt
rm ./cmake_install.cmake

echo "--> Building the given Project (will generate *.gcno files) ..."
cmake .
cmake --build .

echo "--> Cleaning the given directory and Initializing lcov ..."
lcov --zerocounters --directory .
lcov --capture --initial --directory . --output-file app

echo "--> Running Project (will generate *.gcda files) ..."
./$binaryFilename

echo "--> Building lcov Output ..."
lcov --no-checksum --directory . --capture --output-file app.info

echo "--> Generating Html Output ..."
genhtml --highlight --legend --output-directory $genhtmlOutputDir app.info

echo "--> Opening Html Output in Firefox ..."
firefox $genhtmlOutputDir/index.html

echo "DONE."