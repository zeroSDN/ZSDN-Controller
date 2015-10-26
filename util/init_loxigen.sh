echo "LOXIGEN UPGRADE SCRIPT"

ZDIR=$(<zsdn-dir.txt)
echo "ZSDN-Dir: $ZDIR"

cd ~/Downloads

rm -rf loxigen-master

git clone https://github.com/floodlight/loxigen.git loxigen-master

cd loxigen-master
make c -j4

cd loxi_output/loci
if [ "$1" == "-pi" ]; then
	/home/zsdn/raspberrypi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-gcc -fPIC -shared -I inc src/*.c -o libloci.so
else
	gcc -fPIC -shared -I inc src/*.c -o libloci.so
fi
cp -r inc/loci $ZDIR/dependencies/include/
cp libloci.so $ZDIR/dependencies/lib
cd ../../..

rm -rf loxigen-master

rm -rf /home/zsdn/.clion10/system/cmake/generated

cd $ZDIR/dependencies/include/loci/
patch --verbose -i $ZDIR/util/of_match.patch

echo "finished, in clion run file -> invalidate cache (with restart)"