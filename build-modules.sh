#!/bin/bash

# Script for building a module set

ModuleSetPath="modules/module-sets"
CppModulesPath="modules/cpp"
JavaModulesPath="modules/java"


# Scan all module sets
ModuleSets=()
for file in ${ModuleSetPath}/*; do
  ModuleSets+=(${file:20})
done


# Help output
function printHelp {
    echo "Options:"    
    echo "   -m [moduleset] (mandatory) specify module set to build"    
    for i in "${ModuleSets[@]}"
    do
        echo "                  [moduleset] $i"
    done
    echo "   -t             (optional) execute unittests during build";
    echo "   -b [target]>   (optional) set build target, otherwise system default";
    echo "                  [target] pi build for Raspberry Pi ARM"
    echo "   -c             (optional) clean up build output directory before build";
    echo "   -v             (optional) enable verbose output";
}


# Parse Parameters
SkiptTests=true
BuildTarget="default"
ModuleSetSelected=""
Verbose=false
ClearBuildFolder=false

while getopts m:hb:tcv flag; do
  case $flag in
    m)
        ModuleSetSelected="${OPTARG}"
        ;;
    h)
        echo "Build script to build ZSDN modules"
        echo
        printHelp
        exit 0
        ;;
    b)
        echo "-b build target: $OPTARG"
        BuildTarget=$OPTARG
        ;;
    t)
        echo "-t Running with tests"
        SkiptTests=$false
        ;;
    c)
        echo "-c Clearing build folder"
        ClearBuildFolder=true
        ;;
    v)
        # TODO Implement verbose output
        echo "Verbose Output enabled"
        Verbose=true
        ;;
    ?)
        echo "!!Unknown parameter: $flag" 
        ;;
  esac
done


# Check ModuleSetSelected
if [ "$ModuleSetSelected" == "" ] ; then
    echo "!!! No ModuleSetSelected selected"
    printHelp
    exit 1
fi


# Check if existing module set
validModuleSet=false
for i in "${ModuleSets[@]}"
do
    if [[ $ModuleSetSelected =~ $i ]]; then
        validModuleSet=true
        break
    elif [[ "${ModuleSetSelected}.txt" =~ $i ]]; then
        validModuleSet=true
        ModuleSetSelected="${ModuleSetSelected}.txt"
        break
    fi
done

if [ "$validModuleSet" = false ] ; then
    echo "!!! Invalid ModuleSetSelected selected: $ModuleSetSelected"
    echo
    printHelp
    exit 1
fi

echo "Building module set ${ModuleSetSelected}"


# Read modules to build from moduleset
CppModulesToBuild=()
JavaModulesToBuild=()

filename="$ModuleSetPath/$ModuleSetSelected"
while read -r line || [ -n "$line" ]; 
do
    if [[ ${line:0:4} =~ "cpp/" ]]; then
        CppModulesToBuild+=(${line:4})
    elif [[ ${line:0:5} =~ "java/" ]]; then
        JavaModulesToBuild+=(${line:5})
    else
        echo "Unknown module type $i"
    fi
done < "$filename"


# Print modules to build
echo
echo "C++ Modules to build:"
for i in "${CppModulesToBuild[@]}"
do
    echo "   $i"
done

echo "Java Modules to build:"
for i in "${JavaModulesToBuild[@]}"
do
    echo "   $i"
done
echo



# Construct Build Parameters
CommonsBuildArgs=""
CmakeBuildArgs=""
MavenBuildArgs=""

if [ "$SkiptTests" = true ] ; then
    echo "Build skipping Tests"
    CmakeBuildArgs=$CmakeBuildArgs" -DNoTests=ON"
    MavenBuildArgs="$MavenBuildArgs -DskipTests=true"
else
    echo "Build with Tests"
    CommonsBuildArgs=$CommonsBuildArgs" -t"
    CmakeBuildArgs=$CmakeBuildArgs" -DNoTests=OFF"
fi

if [ "$Verbose" = true ] ; then
    echo "Build Verbose"
    CommonsBuildArgs=$CommonsBuildArgs" -v"
    CmakeBuildArgs=$CmakeBuildArgs" -DVerbose=ON"
    # TODO Maven verbosity
fi

if [ "$BuildTarget" = "pi" ] ; then
    echo "Building for RasPi ARM target"
    CmakeBuildArgs=$CmakeBuildArgs" -DPiTarget=ON -DCMAKE_TOOLCHAIN_FILE=$HOME/raspberrypi/pi.cmake"
elif [ "$BuildTarget" = "default" ] ; then
    echo "Build with default target"
else 
    echo "!!Unknown build target: $BuildTarget"   
    exit 1
fi


# Clean up build folder
if [ "$ClearBuildFolder" = true ] ; then
    rm -r "build/modules/$BuildTarget"
    echo "# Cleared output folder"
fi
mkdir -p "build/modules/$BuildTarget"


# Build ZSDN Commons
echo
echo "### Start Build C++ ZSDN Commons ###"
cd common
if ./build-all-common.sh$CommonsBuildArgs; then
    echo "# Build ZSDN Commons Success"
else
    result=$?
    echo "!! Failed to Build ZSDN Commons: "${result}
    exit ${result}
fi
cd ..



# Build C++ modules
echo
echo "### Start Building C++ Modules ###"
cd $CppModulesPath

# Clear 
echo
echo "Clearing C++ Module CMake before build"
for i in "${CppModulesToBuild[@]}"
do
    find . -name CMakeCache.txt -delete
    find . -name Makefile -delete
    find . -name cmake_install.cmake -delete
    find . -name CMakeFiles -type d -exec rm -rf {} +
    echo "# Cleared Module " $i
done
echo "# Cleared Modules"

# Build
echo
echo "Building C++ Modules"
for i in "${CppModulesToBuild[@]}"
do
    echo "# Build " $i
    cd $i
    cmake$CmakeBuildArgs .
    if cmake --build .; then
        echo "# Cmake Module success"
    else
        result=$?
        echo "!! Failed to Cmake Module: "${result}
        exit ${result}
    fi
    cd ..
done

cd ../..
echo "### Finished Build C++ Modules ###"


# Build Java Modules
echo "### Starting Build Java Modules ###"

cd $JavaModulesPath
for i in "${JavaModulesToBuild[@]}"
do
    cd $i
    if ! mvn clean install${MavenBuildArgs}; then
        result=$?
        echo "!! Failed to build java module $i"
        exit ${result}
    fi

    # Copy to module jars to build
    cp target*/*.jar ../../../build/modules/$BuildTarget
    cp target*/*.war ../../../build/modules/$BuildTarget

    cd ..
done

cd ../..
echo "### Finished Build Java Modules ###"


echo "### Finished Build ZSDN Modules ###"