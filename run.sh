# this script is intended to run on c++ projects for the Computer Vision module.
# it will build and then run the project.
# this needs to be run in conjunction with a CMakeLists.txt file.
# this should be placed in the root of your c++ project

# exit the script early on failure
set -e

#!/bin/bash
clear
cd build/
printf "\nBuilding...\n"
cmake ../
make
printf "\nRunning...\n"
./exec
