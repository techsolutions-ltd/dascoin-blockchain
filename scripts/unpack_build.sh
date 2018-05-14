#!/usr/bin/env bash

# The purpose of this script is to unpack the packaged build and prepare it for deployment.

set -o errexit
set -o pipefail
set -o nounset
# set -o xtrace

# Setting magic variables for current file and dir:
__dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
__file="${__dir}/$(basename "${BASH_SOURCE[0]}")"
__base="$(basename ${__file} .sh)"
__root="$(cd "$(dirname "${__dir}")" && pwd)"
__script="$(basename ${BASH_SOURCE[0]})"

numargs=${#}

# Default values:
witness_node_dir_name="witness_node"
cli_wallet_dir_name="cli_wallet"
unpack_cli_wallet=false

# Describe usage of the script:
function usage {
    echo -e "Unpack the packaged blockchain build and prepare it for runtime"\\n
    echo -e "Basic usage: ${__script} <path_to_unpack>"
    echo -e "Optional arguments:"
    echo -e "-c Unpack cli_wallet to path. Deafault is ${unpack_cli_wallet}"
    echo -e "-o -- Name for the witness node directory. Default is ${witness_node_dir_name}"
    echo -e "-l -- Name for the cli wallet directory. Default is ${cli_wallet_dir_name}"
    echo -e ""
    exit 1;
    }

if [ ${numargs} -eq 0 ]; then
    echo -e "ERROR: must supply path to unpack"
    exit 1
fi

# Handle optional flags:
while getopts :o:lch flag; do
  case ${flag} in
    h) usage; exit;;
    o) witness_node_dir_name=${OPTARG};;
    l) cli_wallet_dir_name=${OPTARG};;
    c) unpack_cli_wallet=true;;
    \?) echo "Unknown option: -$OPTARG" >&2; exit 2;;
    : ) echo "Missing option argument for -$OPTARG" >&2; exit 2;;
    * ) echo "Unimplemented option: -$OPTARG" >&2; exit 2;;
  esac
done

# Handle mandatory (unnamed) parameters:
unpack_dir=${1}
shift

echo "Unpacking build:"
echo
echo "Preparing witness node:"
witness_node_dir="${unpack_dir}/${witness_node_dir_name}"
mkdir -p "${witness_node_dir}"
cp "bin/witness_node" "${witness_node_dir}"
cp "config/api_access.json" "${witness_node_dir}"
cp "config/genesis.json" "${witness_node_dir}"
cp "scripts/run_node.sh" "${witness_node_dir}"
cp "scripts/clear.sh" "${witness_node_dir}"

if [ unpack_cli_wallet ]; then
  echo "Preparing cli_wallet:"
  cli_wallet_dir="${unpack_dir}/${cli_wallet_dir_name}"
  mkdir -p "${cli_wallet_dir}"
  cp "bin/cli_wallet" "${cli_wallet_dir}"
  cp "scripts/daemon_run_cli.sh" "${cli_wallet_dir}"
else
  echo "Skipping cli_wallet"
fi

echo "All done!"
