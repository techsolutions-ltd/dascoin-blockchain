#!/bin/sh

DATE=`date +"%d-%m-%Y"`;
FILE_NAME="local-testing-genesis-$DATE.json";

./witness_node --create-genesis-json "$FILE_NAME";
