command_exists () {
    type "$1" &> /dev/null ;
}

echo "LIBTINS UPGRADE SCRIPT"

echo "libtins depends on libpcap. to compile libpcap flex and byacc are required. if the following checks fail, install them, e.g: 'apt-get install flex byacc'"

echo "checking if flex is installed.."
if !  command_exists flex  ; then
	echo "flex is not installed, can't compile libpcap, aborting"
	exit 1
fi
echo "checking if flex is installed.. OK"

echo "checking if byacc is installed.."
if  !  command_exists byacc  ; then
	echo "byacc is not installed, can't compile libpcap, aborting"
	exit 1
fi
echo "checking if byacc is installed.. OK"

ZDIR=$(<zsdn-dir.txt)
echo "ZSDN-Dir: $ZDIR"

cd ~/Downloads

rm -rf libtins
rm -rf libpcap

wget -O libpcap.tar.gz http://www.tcpdump.org/release/libpcap-1.5.3.tar.gz

tar -xzf libpcap.tar.gz
cd libpcap-1.5.3/
if [ "$1" == "-pi" ]; then
	./configure CC=arm-linux-gnueabihf-gcc CXX=arm-linux-gnueabihf-g++ --with-sysroot=/home/raspberrypi/rootfs --host=arm-linux-gnueabihf --prefix=$ZDIR/dependencies
else
	./configure --prefix=$ZDIR/dependencies
fi
make
make install
cd ..
rm -rf libpcap-1.5.3/
rm -rf libpcap.tar.gz

git clone https://github.com/mfontanini/libtins.git libtins
cd libtins
git apply -v $ZDIR/util/libtins_patch.diff
mkdir build
cd build
if [ "$1" == "-pi" ]; then
	cmake -DCMAKE_TOOLCHAIN_FILE=$HOME/raspberrypi/pi.cmake -D_RUN_RESULT_VAR=0 -DCMAKE_INSTALL_PREFIX=$ZDIR/dependencies -DLIBTINS_ENABLE_CXX11=1 -DLIBTINS_ENABLE_WPA2=0 -DLIBTINS_ENABLE_DOT11=0 ..
else
	cmake -DCMAKE_INSTALL_PREFIX=$ZDIR/dependencies -DLIBTINS_ENABLE_CXX11=1 -DLIBTINS_ENABLE_WPA2=0 -DLIBTINS_ENABLE_DOT11=0 ..
fi
make
make install
cd ../..
rm -rf libtins

rm -rf /home/zsdn/.clion10/system/cmake/generated/

echo "finished, in clion run file -> invalidate cache (with restart)"