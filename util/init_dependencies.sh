if [ "$1" == "-pi" ]; then
	echo "init deps for pi"
	bash ../util/init_lemon.sh -pi
	bash ../util/init_libtins.sh -pi
	bash ../util/init_loxigen.sh -pi
else
	echo "init deps for x64"
	bash ../util/init_lemon.sh
	bash ../util/init_libtins.sh
	bash ../util/init_loxigen.sh
fi