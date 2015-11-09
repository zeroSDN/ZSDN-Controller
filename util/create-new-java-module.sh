#!/bin/bash

# Author : Jonas Grunert
# Script for generating new Java modules

echo "### Creating new Java module ###"

# absolute path to root folder of the zsdn controller
zsdnFolder=$(<zsdn-dir.txt)

echo "Enter module name: "
read moduleName

echo "Enter module ID, as 4 hex digits (e.g. "00af"): "
read moduleId

if java -jar module-creator/target/zsdn-module-creator-1.0.0-jar-with-dependencies.jar -i ${moduleId} -n ${moduleName} -d ${zsdnFolder}/modules/java -t ${zsdnFolder}/util/template/java/TemplateModule; then
  echo "# Create Java module success"
else
  result=$?
  echo "!! Failed to create Java module: "${result}
  exit ${result}
fi