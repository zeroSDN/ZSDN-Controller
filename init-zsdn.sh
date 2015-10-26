echo "### Installing ZSDN environment ###"
echo ""
echo "I will try to get it work"
echo "THIS MAY TAKE SOME TIME"

echo ""

ZDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo "Installing to directory: $ZDIR"

echo ""
echo "Press enter to continue (or Ctrl+C to cancel)"
read


cd $ZDIR
echo $ZDIR > ./util/zsdn-dir.txt
echo "set(ZSDN_DIR $ZDIR)" > ./util/zsdn-dir_cmake.txt


echo
echo "## Start init submodules"
git submodule init
git submodule update


cd ZMF
echo
echo "## Start init ZMF"
if ./init-zmf.sh; then
	echo "# init_zmf success"
else
	result=$?
	echo "!! Failed to init_zmf"
	exit ${result}
fi
cd ..


cd util
echo
echo "## Start init ZSDN dependencies"
if ./init_dependencies.sh; then
	echo "# init_dependencies success"
else
	result=$?
	echo "!! Failed to init_dependencies"
	exit ${result}
fi
cd ..


if ./build-all.sh; then
	echo "# Build ALL success"
else
	result=$?
	echo "!! Failed to build ALL"
	exit ${result}
fi

echo "### Finished Its-Not-Working Script ###"