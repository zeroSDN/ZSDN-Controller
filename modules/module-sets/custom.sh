#!/bin/bash

# Module starter script
echo "### Module Starter ###"


trap 'kill $(jobs -p)' EXIT


if [ $# -ne 2 ]; then
	echo "Illegal number of parameters. Usage: "
	echo "<module folder> <config file>"
	echo
	exit 1
fi


# Start modules
# $2 is config file name
#nohup $1/MyCppModule $2 myparam &
#nohup java -jar $1/MyJavaModule.jar $2 myparam &


sleep 0.5
echo 
echo "Modules started"
echo "Press enter to stop controller"
read
