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
nohup $1/SwitchRegistryModule $2 &
nohup $1/DeviceModule $2 &
nohup $1/LinkDiscoveryModule $2 &
nohup $1/TopologyModule $2 &
nohup $1/StatisticsModule $2 &
nohup $1/ARPModule $2 &
nohup $1/SimpleForwardingModule $2 &
nohup $1/ForwardingModule $2 &
nohup java -jar $1/web-admin-standalone.jar &

# Start SwitchAdapter last
$1/SwitchAdapter $2 1.0 6633 
