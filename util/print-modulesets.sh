#!/bin/bash

# Script for loading module sets

ModuleSetPath="modules/module-sets"
CppModulesPath="modules/cpp"
JavaModulesPath="modules/java"


# Scan all module sets
ModuleSets=()
ModuleSetDescriptions=()
for file in ${ModuleSetPath}/*txt; do
	# Load module set
	fileNoExt=${file%%.*}
	setName=${fileNoExt:20}
  	ModuleSets+=($setName)
  	# Load module set description
	setDescr=$(head -n 1 $file)
  	ModuleSetDescriptions+=("${setDescr}")
done

   for i in "${!ModuleSets[@]}"; do 
      echo "                ${ModuleSets[$i]}"
      setDescr=${ModuleSetDescriptions[$i]}
      for ((j = 0 ; j < ${#setDescr} ; j+=57 )); do
        echo "                  ${ModuleSetDescriptions[$i]:j:62}";
      done
  done