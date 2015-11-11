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
modulStarts=(
	"$1/SwitchAdapter $2 1.0 6633"
	"$1/SwitchRegistryModule $2"
	"$1/DeviceModule $2"
	"$1/LinkDiscoveryModule $2"
	"$1/TopologyModule $2"
	"$1/StatisticsModule $2"
	"$1/ARPModule $2"
	"$1/SimpleForwardingModule $2 0"
	"$1/ForwardingModule $2 0")



# Run module starter
java -jar $1/module-starter.jar "${modulStarts[@]}"
