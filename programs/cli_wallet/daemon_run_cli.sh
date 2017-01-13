#!/bin/sh

WALLET_NAME="wallet.json"
NODE_WS_PORT="9880"
NODE_WS_ADDRESS="ws://localhost:$NODE_WS_PORT"
CHAIN_ID="b282ab4e8a3272b191f2747d4c480732da4b07fbddbae6f9dda5ffed73c3a58e"

./cli_wallet --wallet "$WALLET_NAME" -s "$NODE_WS_ADDRESS" --chain-id "$CHAIN_ID" -r 0.0.0.0:6000 -d
