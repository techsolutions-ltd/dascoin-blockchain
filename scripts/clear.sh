#!/usr/bin/env bash

# The purpose of this script is to clean up the blockchain data from the witness node.

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

# Default variables:
data_dir="data"
object_database_dir="object_database"
node_name="witness_node"

function usage {
    echo -e "${__script}: clear the blockchain data and reset the node"\\n
    echo -e "Options:"\\n
    echo -e "-d -- The name of the data directory for the node. Default is ${data_dir}"\\n
    echo -e "-n -- The name of the node executable. Default is ${node_name}"\\n
    echo -e "-h -- Show this message and exit"\\n
}

while getopts d:n:h flag; do
    case ${flag} in
        h) usage; exit;;
        d) data_dir=${OPTARG};;
        n) node_name=${OPTARG};;
        \?) echo "Unknown option: -$OPTARG" >&2; usage; exit 2;;
        : ) echo "Missing option argument for -$OPTARG" >&2; usage; exit 2;;
        * ) echo "Unimplemented option: -$OPTARG" >&2; usage; exit 2;;
    esac
done

shift $((OPTIND-1))  # This tells getopts to move on to the next argument.

if [ ! -e ${node_name} ]; then
    echo "ERROR: node executable ${node_name} does not exist"
    exit
fi

echo "Cleaning up blockchain for witness node in ${__dir}"
rm -rf ${data_dir}
# If the object_database dir persists in the root of the node, delete it.
rm -rf ${object_database_dir}

echo "All done!"

