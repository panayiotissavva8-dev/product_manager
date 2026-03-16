#!/bin/bash

DB_PATH="$(pwd)/build/prodexa.db"

if [ ! -f "$DB_PATH" ]; then
    echo "DB not found, copying from db/prodexa.sqlite"
    cp db/prodexa.sqlite "$DB_PATH"
fi

chmod 664 "$DB_PATH"

./build/product_manager_app