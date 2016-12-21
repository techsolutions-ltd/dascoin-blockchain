#!/bin/sh

WALLET_NAME="wallet.json"
NODE_WS_PORT="9881"
NODE_WS_ADDRESS="ws://localhost:$NODE_WS_PORT"
CHAIN_ID="e6f9f44e4404287c6a4b6da1db76b1fee28572590d882041f33ee02c1932dfdb"

./cli_wallet --wallet "$WALLET_NAME" -s "$NODE_WS_ADDRESS" --chain-id "$CHAIN_ID"
