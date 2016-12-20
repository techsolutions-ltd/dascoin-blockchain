#!/bin/sh

if [ ! -f CMakeCache.txt ]; then
    echo "Cmake cache does not exist, no need to clean. Please run cmake to regenerate build files.";
    return 0;
fi

echo "Cleaning build files:";
make clean;
echo "Removing cmake cache and build files:";
rm -f CMakeCache.txt;
find . -name CMakeFiles | xargs rm -Rf;
echo "All done: please run cmake to regenerate build files.";

