
echo "### Start Build ZSDN Commons ###"


# Parse Parameters
SkiptTests=true
BuildTarget="default"
Verbose=false


while getopts hb:tcv flag; do
  case $flag in
    h)
      echo "--- HELP ---:";
      echo "-t to execute unittests during build";
      echo "-b pi to build for Raspberry Pi ARM";
      exit 0;
      ;;
    b)
      BuildTarget=$OPTARG;
      ;;
    t)
      SkiptTests=$false;
      ;;
    v)
        # TODO Implement verbose output
        echo "Verbose Output enabled"
        Verbose=true
        ;;
    c)
      echo "Clear parameter redundant"
      ;;
    ?)
	  echo "!!Unknown parameter: $flag"	
      ;;
  esac
done


# Build Cpp and Java Topics
cd ../util
./build-topics-cpp.sh
./build-topics-java.sh
./build-proto-cpp.sh
./build-proto-java.sh
cd ../common



# Construct BuildArgs
BuildArgs=""

if [ "$SkiptTests" = true ] ; then
	echo "Build skipping Tests"
	BuildArgs=$BuildArgs" -DNoTests=ON"
else
	echo "Build with Tests"
  BuildArgs=$BuildArgs" -DNoTests=OFF"
fi

if [ "$SkiptTests" = true ] ; then
    echo "Build Verbose"
    BuildArgs=$BuildArgs" -DVerbose=ON"
fi

if [ "$BuildTarget" = "pi" ] ; then
	echo "Building for RasPi ARM target"
	BuildArgs=$BuildArgs" -DCMAKE_TOOLCHAIN_FILE=$HOME/raspberrypi/pi.cmake"
elif [ "$BuildTarget" = "default" ] ; then
	echo "Build with default target"
else 
	echo "!!Unknown build target"	
	exit 1
fi



# Build C++ Commons
echo "# Start Building C++ Commons"
cd cpp

# Clear Commons
find . -name CMakeCache.txt -delete
find . -name Makefile -delete
find . -name cmake_install.cmake -delete
find . -name CMakeFiles -type d -exec rm -rf {} +
echo "# Cleared Commons"


# Búild Commons
cd zsdn-commons
cmake $BuildArgs .
if cmake --build .; then
	echo "# Cmake Common success"
else
	result=$?
	echo "!! Failed to Cmake Common: "${result}
	exit ${result}
fi
cd ../..

echo "# Finished Building C++ Commons"



echo "# Start Building Java Commons"
cd java/zsdn-proto/
if mvn clean install; then
  echo "# mvn install Java Common success"
else
  result=$?
  echo "!! Failed to mvn install Java Common: "${result}
  exit ${result}
fi
cd ../..

echo "# Finished Building Java Commons"


echo "### Finished Build ZSDN Commons ###"