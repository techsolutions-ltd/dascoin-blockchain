#!/usr/bin/env bash

# The purpose of this script is to start up the witness node.

P2P_ADDRESS="0.0.0.0:5770"
BLOCKCHAIN_WS_SERVER="0.0.0.0:9880"

# NOTE: for MASTER nodes you will need to enter the proper witness id and key pair such bash
# --witness-id '"1.6.xxx"'
# --private-key '["GPH.....", "5....."]'
# DO NOT forget the single quotes around the statements!
# For LEDGER nodes remove:
# --enable-stale-production
# --witness-id
# --private-key

./witness_node \
  --data-dir data \
  --genesis-json genesis.json \
  --api-access api_access.json \
  --p2p-endpoint ${P2P_ADDRESS} \
  --rpc-endpoint ${BLOCKCHAIN_WS_SERVER} \
  --enable-stale-production true \
  --witness-id '"1.6.1"' \
  --private-key '["GPH6KXx5gnuZxumkBfHQhZQFPMaca2yAVtjMMhRTLMo4aaiyiKchp","5KZUf3v4PxBSR3NVdEcZpFGJm34bJm9nijm4rYT7S4NSRUeuMd8"]'

