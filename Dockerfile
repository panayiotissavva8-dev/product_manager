# Use lightweight C++ environment
FROM ubuntu:24.04

# Install dependencies
RUN apt-get update && apt-get install -y clang libsqlite3-dev cmake g++ git wget

# Copy project
WORKDIR /app
COPY . .

# Build your app
RUN mkdir build && clang++ source_code/product_manager.cpp -std=c++17 -Iexternal/crow/include \
    -Iexternal/asio/asio/include -lcrypto -lsqlite3 -lpthread -o build/product_manager_app

# Make sure DB exists
RUN mkdir -p build && touch build/prodexa.db

# Expose the port Render will use
ENV PORT=10000
EXPOSE $PORT

# Run the app
CMD ["./build/product_manager_app"]