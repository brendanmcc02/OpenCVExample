# this script removes previous build artifacts

# exit the script early on failure
set -e

#!/bin/bash
clear
rm -rf build/
mkdir build/
printf "Cleaned Previous Build artifacts\n"
