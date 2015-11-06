#!/bin/bash

# Author : Jonas Grunert
# Script for generating new modules

echo "### Creating new C++ module ###"

# absolute path to root folder of the zsdn controller
zsdnFolder=$(<zsdn-dir.txt)

echo "Enter module name: "
read moduleName

echo "Enter module ID, as 4 hex digits (e.g. "00af"): "
read moduleId

if java -jar module-creator/target/zsdn-module-creator-1.0.0-jar-with-dependencies.jar -i ${moduleId} -n ${moduleName} -d ${zsdnFolder}/modules/cpp -t ${zsdnFolder}/util/template/TemplateModule -u ${zsdnFolder}/util/template/TemplateModule-UT; then
  echo "# Create C++ module success"
else
  result=$?
  echo "!! Failed to create C++ module: "${result}
  exit ${result}
fi