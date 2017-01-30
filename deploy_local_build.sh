#!/bin/bash

function check_exists {
    if [[ ! -d "$1" ]]; then
        echo "ERROR: Could not locate $1";
        exit 1;
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
CONFIGS_DIR="/home/paki/graphene-fork/configs/$1"
TARGET_DIR="/home/paki/blockchain-builds/$1_$DATE_TIME"
CLI_WALLET_DIR="$PROGRAM_DIR/cli_wallet"
JS_SERIALIZER_DIR="$PROGRAM_DIR/js_operation_serializer"
WITNESS_NODE_DIR="$PROGRAM_DIR/witness_node"
REMOTE_PATH="repo:/home/repo/shared/blockchain_builds/"

CUR_DIR=`pwd`;

if [[ $# == 0 ]]; then
    echo "ERROR: Missing config directory name";
    exit 1;
fi
echo "Checking if config $1 exists:";
check_exists "$CONFIGS_DIR";

echo "Preparing to deploy blochain witness node and cli wallet for $DATE_TIME"

check_exists "$CLI_WALLET_DIR";
check_exists "$WITNESS_NODE_DIR";
check_exists "$JS_SERIALIZER_DIR";

echo "Creating directory $TARGET_DIR:";
make_dir "$TARGET_DIR"
cd "$TARGET_DIR"

echo "Generating serializers js file:";
/"$JS_SERIALIZER_DIR"/js_operation_serializer | decaffeinate > serializers.js;

echo "Copyining configuration files:"
rsync -qaP "$CONFIGS_DIR"/ .;

echo "Preparing cli_wallet:";
rsync -qaP /"$CLI_WALLET_DIR"/cli_wallet cli_wallet/;

echo "Preparing witness node:";
rsync -qaP "$WITNESS_NODE_DIR"/witness_node witness_node/;

echo "Copyinig chain_id"

# # Copy to repo server:
# # rsync -aPv "$TARGET_DIR" "$REMOTE_PATH";

cd "$CUR_DIR";

echo "All done!";













