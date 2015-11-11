#!/bin/bash

# Script for executing a selected module set

ModuleSetPath="modules/module-sets"
CppModulesPath="modules/cpp"
JavaModulesPath="modules/java"


# Scan all module sets
ModuleSets=()
for file in ${ModuleSetPath}/*txt; do
	# Load module set
	fileNoExt=${file%%.*}
	setName=${fileNoExt:20}
  	ModuleSets+=($setName)
done


# Help output
function printHelp {
    echo "Options:"    
    echo "   -m [set]  (mandatory) Specify module set to execute" 
    ./util/print-modulesets.sh
    echo "   -b [targ] (optional) Set build folder, otherwise system \"default\"";
    echo "                default Build system default"
    echo "                pi      Build for Raspberry Pi ARM"
    echo "   -r        (optional) Rebuild module set before running";
}


# Parse Parameters
BuildParams=""
BuildTarget="default"
ModuleSetSelected=""
Rebuild=false

while getopts m:hb:r flag; do
  case $flag in
    m)
        ModuleSetSelected="${OPTARG}"
        BuildParams="${BuildParams} -m ${OPTARG}"
        ;;
    h)
        echo "Script to execute a ZSDN module set"
        echo
        printHelp
        exit 0
        ;;
    b)
        echo "-b build target: $OPTARG"
        BuildParams="${BuildParams} -b ${OPTARG}"
        BuildTarget=$OPTARG
        ;;
    r)
		echo "Rebuilding module set before running"
		Rebuild=true
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
    fi
done

if [ "$validModuleSet" = false ] ; then
    echo "!!! Invalid ModuleSetSelected selected: $ModuleSetSelected"
    echo
    printHelp
    exit 1
fi


# Rebuild if selected
if [ "$Rebuild" = true ] ; then
echo "### Start building selected modules"
if ./build-modules.sh${BuildParams}; then
	echo "# Build modules success"
else
	result=$?
	echo "!! Failed to build modules"
	exit ${result}
fi
fi


echo "Starting module set ${ModuleSetSelected}"

${ModuleSetPath}/${ModuleSetSelected}.sh "build/modules/${BuildTarget}" "./config/default.config"