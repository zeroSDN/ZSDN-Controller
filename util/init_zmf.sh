#!/bin/bash

cd ../ZMF
echo
echo "## Start init ZMF"
if ./init-zmf.sh; then
	echo "# Init ZMF success"
else
	result=$?
	echo "!! Failed to init ZMF"
	exit ${result}
fi
cd ../util
