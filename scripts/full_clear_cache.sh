#!/usr/bin/env bash

# The purpose of this script is to 

set -o errexit
set -o pipefail
# set -o nounset
# set -o xtrace

# Setting magic variables for current file and dir:
__dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
__file="${__dir}/$(basename "${BASH_SOURCE[0]}")"
__base="$(basename ${__file} .sh)"
__root="$(cd "$(dirname "${__dir}")" && pwd)"
__script="$(basename ${BASH_SOURCE[0]})"

cd "${__root}";  # move to graphene project root

if [ ! -f CMakeCache.txt ]; then
    echo "Cmake cache does not exist, no need to clean. Please run cmake to regenerate build files."
    exit 0
fi

step=0

echo "${step}) Cleaning build files:"
make clean;
echo "Done"
echo
step=$((${step}+1))


echo "${step}) Removing cmake cache and build files:"
rm -f CMakeCache.txt;
find . -name CMakeFiles | xargs rm -Rf;
echo "Done"
echo
step=$((${step}+1))

echo "All done: please run cmake to regenerate build files."

