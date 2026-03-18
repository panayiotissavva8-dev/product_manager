# --- Base image ---
FROM ubuntu:24.04

# --- Environment variables ---
ENV DEBIAN_FRONTEND=noninteractive
ENV APP_DIR=/app
ENV DB_PATH=$APP_DIR/build/prodexa.db
ENV PORT=8080

# --- Install dependencies ---
RUN apt-get update && apt-get install -y \
    clang g++ make cmake git sqlite3 libsqlite3-dev libssl-dev \
    libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

# --- Set working directory ---
WORKDIR $APP_DIR

# --- Copy project files ---
COPY . .

# --- Build app ---
RUN mkdir -p build && \
    clang++ source_code/product_manager.cpp \
    -std=c++17 \
    -Iexternal/crow/include \
    -Iexternal/asio/asio/include \
    -lcrypto -lsqlite3 -lpthread \
    -o build/product_manager_app

# --- Ensure database and tables exist ---
RUN mkdir -p build && \
    sqlite3 $DB_PATH "CREATE TABLE IF NOT EXISTS users(user_id INTEGER PRIMARY KEY AUTOINCREMENT, owner TEXT, username TEXT UNIQUE, password_hash TEXT, role TEXT, termsAccepted INTEGER DEFAULT 0, terms_accepted_at INTEGER DEFAULT 0, email TEXT);" && \
    sqlite3 $DB_PATH "CREATE TABLE IF NOT EXISTS audit(audit_id INTEGER PRIMARY KEY AUTOINCREMENT, owner TEXT, user_id INTEGER, timestamp TEXT, FOREIGN KEY(user_id) REFERENCES users(user_id));" && \
    sqlite3 $DB_PATH "CREATE TABLE IF NOT EXISTS customers(customer_id INTEGER PRIMARY KEY AUTOINCREMENT, owner TEXT NOT NULL, customer TEXT NOT NULL, type TEXT NOT NULL, email TEXT NOT NULL, phone TEXT NOT NULL, address TEXT NOT NULL, postCode TEXT NOT NULL);" && \
    sqlite3 $DB_PATH "CREATE TABLE IF NOT EXISTS products(id INTEGER PRIMARY KEY AUTOINCREMENT, owner TEXT NOT NULL, code INTEGER NOT NULL, brand TEXT NOT NULL, name TEXT NOT NULL, quantity INTEGER DEFAULT 0, stock_alert INTEGER DEFAULT 0, cost REAL DEFAULT 0, price REAL DEFAULT 0, discount REAL DEFAULT 0, vat_amount INTEGER, price_discount REAL, total_price REAL);" && \
    sqlite3 $DB_PATH "CREATE TABLE IF NOT EXISTS sales(sale_code INTEGER PRIMARY KEY AUTOINCREMENT, owner TEXT NOT NULL, customer_id INTEGER NOT NULL, code INTEGER NOT NULL, name TEXT NOT NULL, brand TEXT NOT NULL, quantity INTEGER DEFAULT 0, price REAL DEFAULT 0, discount INTEGER DEFAULT 0, vat_amount INTEGER DEFAULT 0, total_price REAL DEFAULT 0, sum_price REAL NOT NULL, cost REAL NOT NULL, total_cost REAL NOT NULL, date TEXT NOT NULL, FOREIGN KEY(customer_id) REFERENCES customers(customer_id));"

# --- Expose port ---
EXPOSE $PORT

# --- Start the app ---
CMD ["./build/product_manager_app"]