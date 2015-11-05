#!/bin/bash

cd ../JMF
echo
echo "## Start init JMF"
if mvn clean install -DskipTests=true; then
	echo "# Init JMF success"
else
	result=$?
	echo "!! Failed to init JMF"
	exit ${result}
fi
cd ../util
