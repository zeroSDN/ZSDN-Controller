#!/bin/bash

# Module starter script
echo "### Module Starter ###"


# Check parameters
if [ $# -ne 2 ]; then
	echo "Illegal number of parameters. Usage: "
	echo "<module folder> <config file>"
	echo
	exit 1
fi


# Modules to start
# $1 is module folder parameter
# $2 is config file parameter
modulStarts=(
	"$1/MyCppModule $2"
	"java -jar $1/MyJavaModule.jar")


# Run module starter
java -jar $1/module-starter.jar "${modulStarts[@]}"
