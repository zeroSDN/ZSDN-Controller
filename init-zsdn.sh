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
if mvn clean install; then
	echo "# Init hierarchy builder success"
else
	result=$?
	echo "!! Failed to init hierarchy builder"
	exit ${result}
fi
cd ../../..



cd ZMF
echo
echo "## Start init ZMF"
if ./init-zmf.sh; then
	echo "# Init ZMF success"
else
	result=$?
	echo "!! Failed to init ZMF"
	exit ${result}
fi
cd ..


cd util
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
if ./build-all.sh -m zsdn-core; then
	echo "# Build ALL success"
else
	result=$?
	echo "!! Failed to build ALL"
	exit ${result}
fi

echo "### Installing ZSDN environment ###"