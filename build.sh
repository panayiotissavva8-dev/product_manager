#!/bin/bash

echo "Building product_manager_app..."

# Make sure build folder exists
mkdir -p build

# Compile the app
clang++ source_code/product_manager.cpp \
  -std=c++17 \
  -Iexternal/crow/include \
  -Iexternal/asio/asio/include \
  -I/opt/homebrew/opt/openssl@3/include \
  -L/opt/homebrew/opt/openssl@3/lib \
  -lcrypto -lsqlite3 -lpthread \
  -o build/product_manager_app

if [ $? -eq 0 ]; then
    echo "Build complete"
else
    echo "Build failed"
    exit 1
fi