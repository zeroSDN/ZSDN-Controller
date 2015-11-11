#!/bin/bash

# Script for initializing this ZSDN directory

echo "### Installing ZSDN environment ###"
echo ""
echo "Initializing zhe ZSDN environment"
echo "THIS MAY TAKE SOME TIME"
echo 

ZDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo "Installing to directory: $ZDIR"

echo ""
echo "Press enter to continue or wait 10s (or Ctrl+C to cancel)"
read -t 10


cd $ZDIR


echo $ZDIR > ./util/zsdn-dir.txt
echo "set(ZSDN_DIR $ZDIR)" > ./util/zsdn-dir_cmake.txt


echo
echo "## Start init submodules"
git submodule init
git submodule update



echo "## Start Init hierarchy builder"
cd util/hierarchy-builder/hierarchy-builder
if mvn clean install -DskipTests=true; then
	echo "# Init hierarchy builder success"
else
	result=$?
	echo "!! Failed to init hierarchy builder"
	exit ${result}
fi
cd ../../..


echo "## Start Init module creator"
cd util/module-creator
if mvn clean install -DskipTests=true; then
	echo "# Init module creator success"
else
	result=$?
	echo "!! Failed to init module creator"
	exit ${result}
fi
cd ../..


echo "## Start Init module starter"
cd util/module-starter
if mvn clean install -DskipTests=true; then
	echo "# Init module starter success"
else
	result=$?
	echo "!! Failed to init module starter"
	exit ${result}
fi
cd ../..


echo "## Start Init startup selector"
cd util/startup-selector
if mvn clean install -DskipTests=true; then
	echo "# Init startup selector success"
else
	result=$?
	echo "!! Failed to init startup selector"
	exit ${result}
fi
cd ../..


cd util

# Init ZMF
if ! ./init_zmf.sh; then
	result=$?
	echo "!! Failed to init ZMF"
	exit ${result}
fi

# Init JMF
if ! ./init_jmf.sh; then
	result=$?
	echo "!! Failed to init JMF"
	exit ${result}
fi

echo
echo "## Start init ZSDN dependencies"
if ./init_dependencies.sh; then
	echo "# Init ZSDN dependencies success"
else
	result=$?
	echo "!! Failed to init ZSDN dependencies"
	exit ${result}
fi
cd ..


echo
echo "## Start build ZSDN core modules"
if ./build-modules.sh -m zsdn-webadmin; then
	echo "# Build ALL success"
else
	result=$?
	echo "!! Failed to build ALL"
	exit ${result}
fi

echo "### Installing ZSDN environment ###"