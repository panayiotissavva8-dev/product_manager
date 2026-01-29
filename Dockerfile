FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    cmake \
    libsqlite3-dev \
    libssl-dev \
    libasio-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN clang++ product_manager.cpp -std=c++17 \
    -Iexternal/crow/include \
    -lsqlite3 -lcrypto -lpthread \
    -o product_manager_app

EXPOSE 8080
CMD ["./product_manager_app"]
