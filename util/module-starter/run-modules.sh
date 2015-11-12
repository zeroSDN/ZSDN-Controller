#!/bin/bash

# Script to run exported modules

export LD_LIBRARY_PATH="modules/lib"

./modules/starter.sh "./modules" "./config/default.config"
