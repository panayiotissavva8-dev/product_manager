#!/bin/bash

# Absolute path to DB
DB_PATH="$(pwd)/build/prodexa.db"

# Copy DB if missing
if [ ! -f "$DB_PATH" ]; then
    echo "DB not found, copying from db/prodexa.sqlite"
    cp db/prodexa.sqlite "$DB_PATH"
fi

# Make sure permissions are correct
chmod 664 "$DB_PATH"

# Run app
./build/product_manager_app