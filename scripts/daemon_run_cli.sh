#!/usr/bin/env bash

# The purpose of this script is to run the cli_wallet in daemon mode.

BLOCKCHAIN_WS="ws://127.0.0.1:9880"
CLI_WALLET_D_SERVER="0.0.0.0:6000"

./cli_wallet \
  --wallet-file wallet.json \
  --server-rpc-endpoint ${BLOCKCHAIN_WS} \
  --rpc-endpoint ${CLI_WALLET_D_SERVER} \
  --daemon \