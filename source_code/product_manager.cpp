#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>
#include <sqlite3.h>
#include <crow.h>
#include <openssl/sha.h>

using namespace std;

struct Product {
    int code;
    string brand;
    string name;
    int quantity;
    int stock_alert;
    double cost;
    double price;
    double discount;
};

// --- DATABASE FUNCTIONS ---
void loadProducts(sqlite3* db, vector<Product>& products) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT code, brand, name, quantity, stock_alert, cost, price, discount FROM products;";
    products.clear();
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Product p;
        p.code = sqlite3_column_int(stmt, 0);
        p.brand = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        p.quantity = sqlite3_column_int(stmt, 3);
        p.stock_alert = sqlite3_column_int(stmt, 4);
        p.cost = sqlite3_column_double(stmt, 5);
        p.price = sqlite3_column_double(stmt, 6);
        p.discount = sqlite3_column_double(stmt, 7);
        products.push_back(p);
    }
    sqlite3_finalize(stmt);
}

void saveProducts(sqlite3* db, const vector<Product>& products) {
    sqlite3_exec(db, "DELETE FROM products;", nullptr, nullptr, nullptr);

    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO products (code, brand, name, quantity, stock_alert, cost, price, discount) VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    for (const auto& p : products) {
        sqlite3_bind_int(stmt, 1, p.code);
        sqlite3_bind_text(stmt, 2, p.brand.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, p.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, p.quantity);
        sqlite3_bind_int(stmt, 5, p.stock_alert);
        sqlite3_bind_double(stmt, 6, p.cost);
        sqlite3_bind_double(stmt, 7, p.price);
        sqlite3_bind_double(stmt, 8, p.discount);

        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);
}

bool addProduct(const Product& p, vector<Product>& products, sqlite3* db) {
    for (auto& prod : products)
        if (prod.code == p.code) return false;

    products.push_back(p);
    saveProducts(db, products);
    return true;
}

bool deleteProduct(int code, vector<Product>& products, sqlite3* db) {
    auto it = remove_if(products.begin(), products.end(),
                        [&](const Product& p){ return p.code == code; });
    if (it == products.end()) return false;

    products.erase(it, products.end());

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "DELETE FROM products WHERE code = ?;", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, code);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return true;
}

bool updateProduct(const crow::json::rvalue& body, vector<Product>& products, sqlite3* db) {
    int code = body["code"].i();
    
    for (auto& p : products) {
        if (p.code == code) {
            p.brand = body["brand"].s();
            p.name = body["name"].s();
            p.quantity = body["quantity"].i();
            p.stock_alert = body["stock_alert"].i();
            p.cost = body["cost"].d();
            p.price = body["price"].d();
            p.discount = body["discount"].d();

            sqlite3_stmt* stmt;
            const char* sql = "UPDATE products SET brand=?, name=?, quantity=?, stock_alert=?, cost=?, price=?, discount=? WHERE code=?;";
            sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
            sqlite3_bind_text(stmt, 1, p.brand.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, p.name.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 3, p.quantity);
            sqlite3_bind_int(stmt, 4, p.stock_alert);
            sqlite3_bind_double(stmt, 5, p.cost);
            sqlite3_bind_double(stmt, 6, p.price);
            sqlite3_bind_double(stmt, 7, p.discount);
            sqlite3_bind_int(stmt, 8, p.code);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);

            return true;
        }
    }
    return false;
}

string hashPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password.c_str(), password.size(), hash);

    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return ss.str();
}

// --- MAIN ---
int main() {
    // --- DATABASES ---
    sqlite3* db_products;
    sqlite3* db_users;
    sqlite3* db_sales;
    sqlite3* db_audit;

    if (sqlite3_open("databases/products.db", &db_products) != SQLITE_OK) {
        cerr << "Failed to open products.db\n";
        return 1;
    }
    if (sqlite3_open("databases/users.db", &db_users) != SQLITE_OK) {
        cerr << "Failed to open users.db\n";
        return 1;
    }
    if (sqlite3_open("databases/sales.db", &db_sales) != SQLITE_OK) {
        cerr << "Failed to open sales.db\n";
        return 1;
    }
    if (sqlite3_open("databases/user_audit.db", &db_audit) != SQLITE_OK) {
        cerr << "Failed to open user_audit.db\n";
        return 1;
    }

    vector<Product> products;
    loadProducts(db_products, products);

    crow::SimpleApp app;
    string static_folder = "static/";

    // --- ROOT LOGIN ---
    CROW_ROUTE(app, "/")([](){
        ifstream file("static/html/login.html", ios::binary);
        if(!file.is_open()) return crow::response(404, "Cannot open login.html");

        stringstream buffer;
        buffer << file.rdbuf();
        crow::response res(buffer.str());
        res.add_header("Content-Type", "text/html");
        return res;
    });

    // --- REGISTER ---
    CROW_ROUTE(app, "/register")([](){
        ifstream file("static/html/register.html", ios::binary);
        if(!file.is_open()) return crow::response(404, "Cannot open register.html");

        stringstream buffer;
        buffer << file.rdbuf();
        crow::response res(buffer.str());
        res.add_header("Content-Type", "text/html");
        return res;
    });

    // --- DYNAMIC STATIC FILES ---
    app.route_dynamic("/<path>")([&](const crow::request& req, string path){
        ifstream file(static_folder + path, ios::binary);
        if(!file.is_open()) return crow::response(404, "File not found: " + path);

        stringstream buffer;
        buffer << file.rdbuf();
        crow::response res(buffer.str());

        // C++17 safe content type
        if (path.size() >= 4 && path.substr(path.size()-4) == ".css") res.add_header("Content-Type", "text/css");
        else if (path.size() >= 3 && path.substr(path.size()-3) == ".js") res.add_header("Content-Type", "application/javascript");
        else if (path.size() >= 4 && path.substr(path.size()-4) == ".png") res.add_header("Content-Type", "image/png");
        else if (path.size() >= 4 && (path.substr(path.size()-4) == ".jpg" || path.substr(path.size()-5) == ".jpeg")) res.add_header("Content-Type", "image/jpeg");
        else if (path.size() >= 5 && path.substr(path.size()-5) == ".html") res.add_header("Content-Type", "text/html");
        else res.add_header("Content-Type", "application/octet-stream");

        return res;
    });

    // --- LOGIN API ---
    CROW_ROUTE(app, "/login").methods(crow::HTTPMethod::POST)([&db_users](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);

        string username = body["username"].s();
        string password = body["password"].s();
        string input_hash = hashPassword(password);

        sqlite3_stmt* stmt;
        const char* sql = "SELECT password_hash, role FROM users WHERE username = ?;";
        sqlite3_prepare_v2(db_users, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            string db_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            if (db_hash == input_hash) {
                crow::json::wvalue res;
                res["status"] = "success";
                res["role"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                res["username"] = username;
                sqlite3_finalize(stmt);
                return crow::response(res);
            }
        }

        sqlite3_finalize(stmt);
        return crow::response(401, "Invalid credentials");
    });

    // --- REGISTER API ---
    CROW_ROUTE(app, "/add_user").methods(crow::HTTPMethod::POST)([&db_users](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);

        string username = body["username"].s();
        string password = body["password"].s();
        string role = body["role"].s();
        string hashed = hashPassword(password);

        sqlite3_stmt* stmt;
        const char* sql = "INSERT INTO users (username, password_hash, role) VALUES (?, ?, ?);";
        if (sqlite3_prepare_v2(db_users, sql, -1, &stmt, nullptr) != SQLITE_OK)
            return crow::response(500);

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, hashed.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, role.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) return crow::response(409, "User exists");

        sqlite3_finalize(stmt);
        return crow::response(200, "Registered");
    });

    // --- RUN SERVER ---
  int port = 18080;
if (const char* env_p = std::getenv("PORT")) {
    port = std::stoi(env_p);
}
app.port(port).multithreaded().run();

    // --- CLOSE DATABASES ---
    sqlite3_close(db_products);
    sqlite3_close(db_users);
    sqlite3_close(db_sales);
    sqlite3_close(db_audit);

    return 0;
}
