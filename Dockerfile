# Base image
FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    cmake \
    libsqlite3-dev \
    libssl-dev \
    git \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy all files
COPY . .

# Build the C++ application
RUN clang++ product_manager.cpp -std=c++17 \
    -Iexternal/crow/include \
    -l sqlite3 -lcrypto -lpthread -o product_manager_app

# Expose the port Crow will listen on
EXPOSE 18080

# Run the app
CMD ["./product_manager_app"]
