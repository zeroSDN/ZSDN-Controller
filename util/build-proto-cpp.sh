#!/bin/bash

# Author : Andre Kutzleb
# Script for generating factory code for  protobuf-messages
# for all zsdn modules.
#
# requires protoc 2.6.1 installed.

echo "### Building C++ Protobuf files ###"

# absolute path to root folder of the zsdn controller
zsdnFolder=$(<zsdn-dir.txt)

protoc="$zsdnFolder/ZMF/dependencies/bin/protoc"
requiredProtocVersion="libprotoc 2.6.1"

protocVersion=$($protoc --version)


# Create proto out dir
mkdir $zsdnFolder/common/cpp/zsdn-commons/zsdn/proto/


tempFolder="$zsdnFolder/util/.temp/"


if [ "$requiredProtocVersion" != "$protocVersion" ]; then
	echo "output of protoc --version ($protocVersion) does not match the required protoc version ($requiredProtocVersion). please install the required protoc version."
	exit 1
fi

mkdir $tempFolder

protosRaw=$(find $zsdnFolder/modules -name '*.proto')

for i in $protosRaw
do
	cp $i "$tempFolder"
done

protosInTemp=$(find $tempFolder -name '*.proto')

for i in $protosInTemp
do
	echo "Processing $i";
	$protoc $i "--proto_path=$tempFolder" "--cpp_out=$zsdnFolder/common/cpp/zsdn-commons/zsdn/proto"
done

rm -r "$tempFolder"

echo "### Finished Building C++ Protobuf files ###"
