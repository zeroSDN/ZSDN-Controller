echo "LEMON UPGRADE SCRIPT"

ZDIR=$(<zsdn-dir.txt)
echo "ZSDN-Dir: $ZDIR"

cd ~/Downloads

wget -O lemon.tar.gz http://lemon.cs.elte.hu/pub/sources/lemon-1.3.1.tar.gz

rm -rf lemon-1.3.1

tar -xzf lemon.tar.gz

cd lemon-1.3.1
mkdir build
cd build
if [ "$1" == "-pi" ]; then
	cmake -DCMAKE_TOOLCHAIN_FILE=$HOME/raspberrypi/pi.cmake -DCMAKE_INSTALL_PREFIX=$ZDIR/dependencies ..
else
	cmake -DCMAKE_INSTALL_PREFIX=$ZDIR/dependencies ..
fi
make -j4
make install
cd ../..

rm -rf lemon-1.3.1

rm lemon.tar.gz

rm -rf /home/zsdn/.clion10/system/cmake/generated/

echo "finished, in clion run file -> invalidate cache (with restart)"