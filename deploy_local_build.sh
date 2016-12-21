#!/bin/sh

function check_exists {
    if [[ ! -d "$1" ]]; then
        echo "ERROR: Could not locate $1";
        return 1;
    fi
}

function make_dir {
    mkdir -p "$1"
    if [[ $? != 0 ]]; then
        echo "Could not create build directory $1";
        cd "$CUR_DIR";
        return 1;
    fi
}

function copy_dir {
    rsync -aPv "$1" .;
    if [[ $? != 0 ]]; then
        echo "Failed to copy dir $1 to $PWD";
        return 1;
    fi
}

DATE_TIME=`date +"%d-%m-%YT%H-%M-%S"`;
PROGRAM_DIR="/home/paki/graphene-fork/programs"
TARGET_DIR="/home/paki/blockchain-builds/$DATE_TIME"
CLI_WALLET_DIR="$PROGRAM_DIR/cli_wallet"
WITNESS_NODE_DIR="$PROGRAM_DIR/witness_node"
REMOTE_PATH="repo:/home/repo/shared/blockchain_builds/"

CUR_DIR=`pwd`;

echo "Preparing to deploy blochain witness node and cli wallet for $DATE_TIME"

check_exists "$CLI_WALLET_DIR";
check_exists "$WITNESS_NODE_DIR";

echo "Creating directory $TARGET_DIR";
make_dir "$TARGET_DIR"
cd "$TARGET_DIR"

echo "Creating directory for cli_wallet";
copy_dir "$CLI_WALLET_DIR";
cd "cli_wallet";
echo "Removing external files from $PWD";
# Clean up cli_wallet:
# Remove backup wallet files:
rm -rf *.wallet;
rm -rf  CMake*;
rm -rf  *.cmake;
rm -rf  *.cpp;
rm -rf  Makefile;
rm -rf  nuke_cli_wallet.sh;

cd ..;

echo "Creating directory for witness node";
copy_dir $WITNESS_NODE_DIR;
cd "witness_node";
echo "Removing external files from $PWD";
#Clean up witness_node
rm -rf  CMake*;
rm -rf  *.cmake;
rm -rf  *.cpp;
rm -rf  Makefile;

cd ..;

# Copy to repo server:
scp -r "$TARGET_DIR" "$REMOTE_PATH";

cd "$CUR_DIR";













