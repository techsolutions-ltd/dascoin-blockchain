#!/bin/sh

WALLET_NAME="wallet.json"
NODE_WS_PORT="9880"
NODE_WS_ADDRESS="ws://localhost:$NODE_WS_PORT"

./cli_wallet --wallet "$WALLET_NAME" -s "$NODE_WS_ADDRESS" -r 0.0.0.0:6000 -d
