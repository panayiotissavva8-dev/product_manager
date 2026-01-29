FROM ubuntu:22.04

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    libsqlite3-dev \
    libssl-dev \
    libasio-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# App directory
WORKDIR /app

# Copy source and dependencies
COPY src ./src
COPY external ./external

# Build
RUN clang++ src/product_manager.cpp -std=c++17 \
    -DASIO_STANDALONE \
    -Iexternal/crow/include \
    -lsqlite3 -lcrypto -lpthread \
    -o product_manager_app

# Render uses PORT env var
EXPOSE 8080

# Run
CMD ["./product_manager_app"]
