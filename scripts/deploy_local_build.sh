#!/usr/bin/env bash

# The purpose of this script is to deploy the local build of the blockchain to a defined target directory.
# The script will use a configuration specified in the config folder.

set -o errexit
set -o pipefail
# set -o nounset
# set -o xtrace

function check_dir_exists {
    if [[ ! -d "${1}" ]]; then
        echo "ERROR: could not locate directory ${1}";
        exit 1;
    fi
}

function make_dir {
    mkdir -p "${1}"
    if [[ $? != 0 ]]; then
        echo "ERROR: could not create directory ${1}";
        exit 1;
    fi
}

function copy {
    rsync -qaP $1 $2
    if [ $? -ne 0 ]; then
        echo "ERROR: could not copy directory ${1} to directory ${2}"
        exit 1
    fi
}

# Compress folder <archive_path> <folder_path> <target_path>
function compress_folder {
    tar zcf "${1}.tar.gz" -C "${2}" .
    if [ $? -ne 0 ]; then
        echo "ERROR: could not compress directory ${2}"
        exit 1
    fi
    check_dir_exists "${3}"
    mv "${1}.tar.gz" "${3}"
}

# Return <string> with build type: "Debug"/"Release"
function get_cmake_build_type {
    grep CMAKE_BUILD_TYPE ${__root}/CMakeCache.txt | awk -F"=" '{print $2}'
}

# Setting magic variables for current file and dir:
__dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
__file="${__dir}/$(basename "${BASH_SOURCE[0]}")"
__base="$(basename ${__file} .sh)"
__root="$(cd "$(dirname "${__dir}")" && pwd)"
__script="$(basename ${BASH_SOURCE[0]})"

# Set default variables:
config_name="testing"
program_dir="${__root}/programs"
configs_root="${__root}/configs"
target_dir="${HOME}/blockchain-builds"
remote_dir=""
compress=false

#Set fonts:
# norm_font=`tput sgr0`
# bold_font=`tput bold`
# rev_font=`tput smso`

# Describe usage of the script:
function help {
    echo -e \\n"Copy the local blockchain build to a staging deployment."\\n
    echo -e "Basic usage: ${bold_font}${__script} [-t <string> | -c <string>]${norm_font}"\\n
    echo -e "Command line switches are optional. The following switches are recognized:"\\n
    echo -e "${bold_font}-t${norm_font}  --Target directory to which to deploy the build. Default is ${bold_font}${target_dir}${norm_font}"
    echo -e "${bold_font}-c${norm_font}  --Configuration name. Default is ${bold_font}${config_name}${norm_font}"
    echo -e "${bold_font}-z${norm_font}  --Create a compressed build folder. Default is ${bold_font}false${norm_font}"
    echo -e "${bold_font}-s${norm_font}  --Copy the compressed build to a remote location. Activates ${bold_font}-z${norm_font}"
    echo -e "${bold_font}-h${norm_font}  --Displays this message."\\n
    exit 1;
    }

numargs=$#

while getopts :c:t:s:zh flag; do
  case $flag in
    c)
      config_name=$OPTARG
      ;;
    t)
      target_dir=$OPTARG
      ;;
    z)
      compress=true;
      ;;
    s)
      compress=true;
      remote_dir=$OPTARG
      ;;
    h)
      help
      ;;
    \?) #unrecognized option - show help
      echo -e \\n"Option -${bold_font}$OPTARG${norm_font} not allowed."
      echo -e "Use ${bold_font}$__script -h${norm_font} to see the help documentation."\\n
      exit 2
      ;;
  esac
done

shift $((OPTIND-1))  #This tells getopts to move on to the next argument.

# Test for mandatory parameters:


config_dir="${configs_root}/${config_name}"

# Check if configuration directory exists:
check_dir_exists ${config_dir};

# Form build name:
get_cmake_build_type
build_name="$(git describe)_${config_name}_$(get_cmake_build_type)"
build_dir="${target_dir}/${build_name}"

step=0

echo "Deploying build ${build_name} to ${target_dir}:"

echo "${step}) Copying ${config_name} configuration:"
make_dir "${build_dir}"
copy "${config_dir}/" "${build_dir}"
echo "Done"
echo
step=$((${step}+1))

echo "${step}) Creating serializers file:"
"${program_dir}"/js_operation_serializer/js_operation_serializer | decaffeinate > ${build_dir}/serializers.js;
echo "Done"
echo
step=$((${step}+1))

echo "${step}) Copying witness node executable:"
copy "${program_dir}/witness_node/witness_node" "${build_dir}/witness_node/witness_node"
echo "Done"
echo
step=$((${step}+1))

echo "${step}) Copying cli_wallet node executable:"
copy "${program_dir}/cli_wallet/cli_wallet" "${build_dir}/cli_wallet/cli_wallet"
echo "Done"
echo
step=$((${step}+1))

if [ ${compress} = true ]; then
    echo "${step}) Compressing build folder:"
    build_archive_name="${target_dir}/${build_name}.tar.gz"
    compress_folder "${build_name}" "${build_dir}" "${target_dir}"
    echo "Done"
    echo
    step=$((${step}+1))
fi

if [ -n "${remote_dir}" ]; then
    echo "${step}) Copyinig compressed build to remote ${remote_dir}:"
    copy "${target_dir}/${build_name}.tar.gz" "${remote_dir}"
    echo "Done"
    echo
    step=$((${step}+1))
fi

echo "${step}) Creating symbolic link for last build"
ln -fs ${build_dir} ${target_dir}/last_build
echo "Done"
echo

echo "All done!";
