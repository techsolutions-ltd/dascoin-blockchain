#!/usr/bin/env bash

# The purpose of this script is to package the build files for deployment.

set -o errexit
set -o pipefail
# set -o nounset
# set -o xtrace

# Return <string> with build type: "Debug"/"Release"
function get_cmake_build_type {
    grep CMAKE_BUILD_TYPE ${build_dir}/CMakeCache.txt | awk -F"=" '{print $2}'
}

# Setting magic variables for current file and dir:
__dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
__file="${__dir}/$(basename "${BASH_SOURCE[0]}")"
__base="$(basename ${__file} .sh)"
__root="$(cd "$(dirname "${__dir}")" && pwd)"
__script="$(basename ${BASH_SOURCE[0]})"

# Default variables:
build_dir="${__dir}/../build"
config_dir="${__dir}/../configs"
scripts_dir="${__dir}/../scripts"
config_name="testing"
target_dir="${__dir}/.."
remote_dir=""
compress=false

numargs=$#

# Describe usage of the script:
function help {
    echo -e \\n"Package the build and prepare for deployment."\\n
    echo -e "Basic usage: ${__script} -r <string> [-b <string> | -c <string> | -z | -h ]"\\n
    echo -e "Mandatory command line switches:"\\n
    echo -e "-r --Remote directory to which to move packaged build"\\n
    echo -e "Optional command line switches:"\\n
    echo -e "-b  --Build directory. Default is ${build_dir}"
    echo -e "-c  --Configuration name. Default is ${config_name}"
    echo -e "-z  --Create a compressed build folder. Default is false"
    echo -e "-h  --Displays this message."\\n
    exit 1;
    }

while getopts :r:c:z:bh flag; do
  case ${flag} in
    h) help; exit;;
    c) config_name=${OPTARG};;
    r) remote_dir=${OPTARG};;
    z) compress=true;;
    b) build_dir=${OPTARG};;
    \?) echo "Unknown option: -$OPTARG" >&2; exit 2;;
    : ) echo "Missing option argument for -$OPTARG" >&2; exit 2;;
    * ) echo "Unimplemented option: -$OPTARG" >&2; exit 2;;
  esac
done

# Check mandatory arguments:
if [ -z ${remote_dir} ]; then
    echo "Must include remote dir (option -r)"; exit 2;
fi

shift $((OPTIND-1))  # This tells getopts to move on to the next argument.

step=1

build_type=$(get_cmake_build_type)
build_name="$(git describe)_${config_name}_${build_type,,}"  # Convert to lowercase.
target_dir="${target_dir}/${build_name}"

echo "Packaging build ${build_name}:"
echo

echo "${step}) Making target directory ${target_dir}"
mkdir -p "${target_dir}/bin"
mkdir -p "${target_dir}/config"
mkdir -p "${target_dir}/scripts"
echo "Done"
echo
step=$((${step}+1))

echo "${step}) Copying witness_node binary:"
cp "${build_dir}/programs/witness_node/witness_node" "${target_dir}/bin"
echo "Done"
echo
step=$((${step}+1))

echo "${step}) Copying cli_wallet binary:"
cp "${build_dir}/programs/cli_wallet/cli_wallet" "${target_dir}/bin"
echo "Done"
echo
step=$((${step}+1))

echo "${step}) Copying the \"${config_name}\" configuration:"
config_dir="${config_dir}/${config_name}"
cp -r "${config_dir}/." "${target_dir}/config"
echo "Done"
echo
step=$((${step}+1))

echo "${step}) Copying the scripts:"
scritps_target_dir="${target_dir}/scripts"
cp "${scripts_dir}/clear.sh" "${scritps_target_dir}/"
cp "${scripts_dir}/run_node.sh" "${scritps_target_dir}/"
cp "${scripts_dir}/daemon_run_cli.sh" "${scritps_target_dir}/"
# The unpack script is in the root directory:
cp "${scripts_dir}/unpack_build.sh" "${target_dir}/"
echo "Done"
echo
step=$((${step}+1))

echo "${step}) Copying packaged build to remote folder ${remote_dir}:"
cp -r "${target_dir}/" "${remote_dir}"
rm -rf "${target_dir}"
echo "Done"
echo
step=$((${step}+1))

if [ ${compress} = true ]; then
    echo "${step}) Compressing build folder:"
    tar zcfv "${build_name}.tar.gz" -C "${remote_dir}" "${build_name}"
    mv "${build_name}.tar.gz" "${remote_dir}"
    echo "Done"
    echo
    step=$((${step}+1))
fi

echo "All done!"
