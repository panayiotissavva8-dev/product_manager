#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sqlite3.h>
#include <crow.h>
#include <openssl/sha.h>
#include <unordered_map>
#include <string>
#include <ctime>

// Global session map: token -> username
std::unordered_map<std::string, std::string> sessions;


using namespace std;

struct Product {
    string owner;
    int code;
    string brand;
    string name;
    int quantity;
    int stock_alert;
    double cost;
    double price;
    double discount;
};

struct sale {
    string owner;
    int code;
    string brand;
    string name;
    int quantity;
    double price;
    double discount;
    double total_price;
    string date;
};

// --- UTILITY FUNCTIONS ---
// Get current date and time
std::string getCurrentDateTime() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
    localtime_r(&t, &tm); 

    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    return buffer;
}


// --- DATABASE FUNCTIONS ---
void loadProducts(sqlite3* db, vector<Product>& products, const string& username) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT code, brand, name, quantity, stock_alert, cost, price, discount FROM products WHERE owner = ?;";
    products.clear();
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
         Product p;
        p.owner = username;
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
    const char* sql = "INSERT INTO products (owner, code, brand, name, quantity, stock_alert, cost, price, discount) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    for (const auto& p : products) {
        sqlite3_bind_text(stmt, 1, p.owner.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, p.code);
        sqlite3_bind_text(stmt, 3, p.brand.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, p.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 5, p.quantity);
        sqlite3_bind_int(stmt, 6, p.stock_alert);
        sqlite3_bind_double(stmt, 7, p.cost);
        sqlite3_bind_double(stmt, 8, p.price);
        sqlite3_bind_double(stmt, 9, p.discount);

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

bool updateProduct(const crow::json::rvalue& body, vector<Product>& products, sqlite3* db, const string& username) {
    int code = body["code"].i();
    for (auto& p : products) {
        if (p.code == code && p.owner == username) {
            p.brand = body["brand"].s();
            p.name = body["name"].s();
            p.quantity = body["quantity"].i();
            p.stock_alert = body["stock_alert"].i();
            p.cost = body["cost"].d();
            p.price = body["price"].d();
            p.discount = body["discount"].d();
            saveProducts(db, products);
            return true;
        }
    }
    return false;
}

bool deleteProduct(int code, vector<Product>& products, sqlite3* db, const string& username) {
    auto it = remove_if(products.begin(), products.end(),
     [&](const Product& p){ return p.code == code && p.owner == username; });
    if (it == products.end()) return false;

    products.erase(it, products.end());
    saveProducts(db, products);
    return true;
}

string hashPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password.c_str(), password.size(), hash);

    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return ss.str();
}

void loadSales(sqlite3* db, vector<sale>& sales, const string& username) {
    sales.clear();
    sqlite3_stmt* stmt;
    // Case-insensitive username match
    const char* sql = "SELECT code, brand, name, quantity, price, discount, total_price, date "
                  "FROM sales WHERE LOWER(owner) = LOWER(?);";

    
                      cout << "Loading sales for user: '" << username << "'" << endl;


    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare loadSales stmt: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    cout << "Executing query for username: '" << username << "'" << endl;


    while (sqlite3_step(stmt) == SQLITE_ROW) {
        sale s;
        s.owner = username;
        s.code = sqlite3_column_int(stmt, 0);
        s.brand = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        s.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        s.quantity = sqlite3_column_int(stmt, 3);
        s.price = sqlite3_column_double(stmt, 4);
        s.discount = sqlite3_column_double(stmt, 5);
        s.total_price = sqlite3_column_double(stmt, 6);
        const unsigned char* date_text = sqlite3_column_text(stmt, 7);
        s.date = date_text ? reinterpret_cast<const char*>(date_text) : "";
        sales.push_back(s);
    }
    cout << "Total sales loaded: " << sales.size() << endl;


    sqlite3_finalize(stmt);
}


void saveSales(sqlite3* db, const vector<sale>& sales) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO sales (owner, code, brand, name, quantity, price, discount, total_price, date) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare saveSales stmt: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    for (const auto& s : sales) {
        sqlite3_bind_text(stmt, 1, s.owner.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, s.code);
        sqlite3_bind_text(stmt, 3, s.brand.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, s.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 5, s.quantity);
        sqlite3_bind_double(stmt, 6, s.price);
        sqlite3_bind_double(stmt, 7, s.discount);
        sqlite3_bind_double(stmt, 8, s.total_price);

        // Use the date from the object; fallback to now if empty
        std::string date_to_save = s.date.empty() ? getCurrentDateTime() : s.date;
        sqlite3_bind_text(stmt, 9, date_to_save.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;
        }

        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
    }

    sqlite3_finalize(stmt);
}




int main() {
    // --- DATABASES ---
    sqlite3* db_products;
    sqlite3* db_users;
    sqlite3* db_sales;

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
    cout << "SALES DB FILE: "
     << sqlite3_db_filename(db_sales, "main")
     << endl;



   // string username = sessions[token];

   // vector<sale> sales;
  //  loadSales(db_sales, sales, username);

    vector<Product> products;
    // loadProducts(db_products, products, username);

    crow::SimpleApp app;


    // --- LOGIN API ---
    CROW_ROUTE(app, "/login").methods(crow::HTTPMethod::POST)([&db_users](const crow::request& req){
    auto body = crow::json::load(req.body);
    if (!body) return crow::response(400);

    std::string username = body["username"].s();
    std::string password = body["password"].s();
    std::string input_hash = hashPassword(password);

    sqlite3_stmt* stmt;
    const char* sql = "SELECT password_hash, role FROM users WHERE username = ?;";
    sqlite3_prepare_v2(db_users, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string db_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        if (db_hash == input_hash) {
            // generate simple session token
            std::string token = username + "_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            sessions[token] = username;

            crow::json::wvalue res;
            res["status"] = "success";
            res["username"] = username;
            res["role"] = role;
            res["token"] = token;  // send token to frontend
            sqlite3_finalize(stmt);
            return crow::response(200, res);
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
        bool termsAccepted = body["termsAccepted"].b();
        string hashed = hashPassword(password);
        time_t terms_accepted_at = time(nullptr);

         if (!termsAccepted) {
            return crow::response(400, "Terms must be accepted");
        }
        sqlite3_stmt* stmt;
        const char* sql = "INSERT INTO users (username, password_hash, role, termsAccepted, terms_accepted_at) VALUES (?, ?, ?, ?, ?);";
        if (sqlite3_prepare_v2(db_users, sql, -1, &stmt, nullptr) != SQLITE_OK)
            return crow::response(500);

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, hashed.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, role.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, termsAccepted ? 1 : 0);
        sqlite3_bind_int64(stmt, 5, static_cast<sqlite3_int64>(terms_accepted_at));

        if (sqlite3_step(stmt) != SQLITE_DONE) return crow::response(409, "User exists");

        sqlite3_finalize(stmt);
        return crow::response(200, "Registered");
    });

   // --- LOG OUT API ---
CROW_ROUTE(app, "/logout").methods(crow::HTTPMethod::POST)([](const crow::request& req){
    auto token = req.get_header_value("Authorization");

    if (!token.empty() && sessions.find(token) != sessions.end()) {
        sessions.erase(token);
    }

    crow::json::wvalue res;
    res["status"] = "success";
    return crow::response(200, res); 
});



    // --- CURRENT USER API ---
CROW_ROUTE(app, "/current_user")([&db_users](const crow::request& req){
    auto token = req.get_header_value("Authorization");
    if (token.empty() || sessions.find(token) == sessions.end())
        return crow::response(401, "Not logged in");

    std::string username = sessions[token];

    sqlite3_stmt* stmt;
    const char* sql = "SELECT role FROM users WHERE username = ?;";
    sqlite3_prepare_v2(db_users, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    std::string role = "user";
    if (sqlite3_step(stmt) == SQLITE_ROW)
        role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

    sqlite3_finalize(stmt);

    crow::json::wvalue res;
    res["status"] = "success";
    res["username"] = username;
    res["role"] = role;
    return crow::response(200, res);
});




  // --- PRODUCTS DATA API ---
CROW_ROUTE(app, "/api/products")
([&products, &db_products](const crow::request& req){
    auto token = req.get_header_value("Authorization");
if (token.empty() || sessions.find(token) == sessions.end())
    return crow::response(401, "Not logged in");

string username = sessions[token];

loadProducts(db_products, products, username);


    crow::json::wvalue out;
    out["products"] = crow::json::wvalue::list();

    int i = 0;
    for (const auto& p : products) {
      //  if (p.owner != username) continue; // only return products of the logged-in user
        crow::json::wvalue obj;
        obj["code"] = p.code;
        obj["brand"] = p.brand;
        obj["name"] = p.name;
        obj["quantity"] = p.quantity;
        obj["stock_alert"] = p.stock_alert;
        obj["cost"] = p.cost;
        obj["price"] = p.price;
        obj["discount"] = p.discount;
        out["products"][i++] = std::move(obj);
    }

    return crow::response(200, out);
});

    // --- GET PRODUCT API ---
CROW_ROUTE(app, "/get_product")
([&products, &db_products](const crow::request& req){
    auto query_param = req.url_params.get("query");
    if (!query_param) return crow::response(400, "Missing query");

    string query = query_param;

    auto token = req.get_header_value("Authorization");
if (token.empty() || sessions.find(token) == sessions.end())
    return crow::response(401, "Not logged in");

string username = sessions[token];

loadProducts(db_products, products, username);


    for (const auto& p : products) {
      //  if (p.owner != username) continue; // only search products of the logged-in user
        string name_lower = p.name;
        string query_lower = query;
        // convert both to lowercase for case-insensitive comparison
        transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
        transform(query_lower.begin(), query_lower.end(), query_lower.begin(), ::tolower);

        string code_str = to_string(p.code);

        if (name_lower.find(query_lower) != string::npos || code_str.find(query) != string::npos) {
            crow::json::wvalue res;
            res["code"] = p.code;
            res["brand"] = p.brand;
            res["name"] = p.name;
            res["quantity"] = p.quantity;
            res["stock_alert"] = p.stock_alert;
            res["cost"] = p.cost;
            res["price"] = p.price;
            res["discount"] = p.discount;
            return crow::response(200, res);
        }
    }

    return crow::response(404, "Product not found");
});

// --- Sales Data API ---
CROW_ROUTE(app, "/api/sales")
([ &db_sales](const crow::request& req){
    auto token = req.get_header_value("Authorization");
if (token.empty() || sessions.find(token) == sessions.end())
    return crow::response(401, "Not logged in");

string username = sessions[token];
transform(username.begin(), username.end(), username.begin(), ::tolower);

vector<sale> sales;
 loadSales(db_sales, sales, username);
 cout << "Sales loaded: " << sales.size() << endl;
for (auto& s : sales) {
    cout << s.code << " " << s.name << " " << s.total_price << " " << s.date << endl;
} 
cout << "SESSION USERNAME = [" << username << "]" << endl;


    crow::json::wvalue out;
    out["sales"] = crow::json::wvalue::list();

    int i = 0;
    for (const auto& s : sales) {
     //   if (p.owner != username) continue; // only return sales of the logged-in user
        crow::json::wvalue obj;
        obj["user"] = s.owner;
        obj["code"] = s.code;
        obj["brand"] = s.brand;
        obj["name"] = s.name;
        obj["quantity"] = s.quantity;
        obj["price"] = s.price;
        obj["discount"] = s.discount;
        obj["total_price"] = s.total_price;
        obj["date"] = s.date;
        out["sales"][i++] = std::move(obj);
    }

    return crow::response(200, out);
});






   string static_folder = "static/";

// --- ROOT LANDING PAGE ---
   CROW_ROUTE(app, "/")([](){
        ifstream file("static/html/landing.html", ios::binary);
        if(!file.is_open()) return crow::response(404, "Cannot open landing.html");

        stringstream buffer;
        buffer << file.rdbuf();
        crow::response res(buffer.str());
        res.add_header("Content-Type", "text/html");
        return res;
    });

    // --- LOGIN PAGE ---
    CROW_ROUTE(app, "/login")([](){
        ifstream file("static/html/login.html", ios::binary);
        if(!file.is_open()) return crow::response(404, "Cannot open login.html");

        stringstream buffer;
        buffer << file.rdbuf();
        crow::response res(buffer.str());
        res.add_header("Content-Type", "text/html");
        return res;
    });

    // --- REGISTER PAGE ---
    CROW_ROUTE(app, "/register")([](){
        ifstream file("static/html/register.html", ios::binary);
        if(!file.is_open()) return crow::response(404, "Cannot open register.html");

        stringstream buffer;
        buffer << file.rdbuf();
        crow::response res(buffer.str());
        res.add_header("Content-Type", "text/html");
        return res;
    });

    // --- DASHBOARD PAGE ---
CROW_ROUTE(app, "/dashboard")([](){
    ifstream file("static/html/dashboard.html", ios::binary);
    if(!file.is_open()) return crow::response(404, "dashboard.html not found");

    stringstream buffer;
    buffer << file.rdbuf();
    crow::response res(buffer.str());
    res.add_header("Content-Type", "text/html");
    return res;
});

// --- PRODUCTS PAGES ---

// --- VIEW PRODUCTS PAGE ---
CROW_ROUTE(app, "/products")([](){
    ifstream file("static/html/view_products.html", ios::binary);
    if (!file.is_open())
        return crow::response(404, "view_products.html not found");

    stringstream buffer;
    buffer << file.rdbuf();
    crow::response res(buffer.str());
    res.add_header("Content-Type", "text/html");
    return res;
});

// --- ADD PRODUCT PAGE ---
CROW_ROUTE(app, "/add_product")([](){
    ifstream file("static/html/add_product.html", ios::binary);
    if (!file.is_open())
        return crow::response(404, "add_product.html not found");

    stringstream buffer;
    buffer << file.rdbuf();
    crow::response res(buffer.str());
    res.add_header("Content-Type", "text/html");
    return res;
});

// --- EDIT PRODUCT PAGE ---
CROW_ROUTE(app, "/edit_product")([](){
    ifstream file("static/html/edit_product.html", ios::binary);
    if (!file.is_open())
        return crow::response(404, "edit_product.html not found");

    stringstream buffer;
    buffer << file.rdbuf();
    crow::response res(buffer.str());
    res.add_header("Content-Type", "text/html");
    return res;
});

// --- SALES PAGE ---

// --- VIEW SALES PAGE ---
CROW_ROUTE(app, "/sales")([](){
    ifstream file("static/html/view_sales.html", ios::binary);
    if (!file.is_open())
        return crow::response(404, "view_sales.html not found");

    stringstream buffer;
    buffer << file.rdbuf();
    crow::response res(buffer.str());
    res.add_header("Content-Type", "text/html");
    return res;
});

// --- ADD SALE PAGE ---
CROW_ROUTE(app, "/add_sale")([](){
    ifstream file("static/html/add_sale.html", ios::binary);
    if (!file.is_open())
        return crow::response(404, "add_sale.html not found");

    stringstream buffer;
    buffer << file.rdbuf();
    crow::response res(buffer.str());
    res.add_header("Content-Type", "text/html");
    return res;
});



    // --- STATIC FILES ---
    app.route_dynamic("/<path>")([&](const crow::request& req, string path){
        ifstream file(static_folder + path, ios::binary);
        if(!file.is_open()) return crow::response(404, "File not found: " + path);

        stringstream buffer;
        buffer << file.rdbuf();
        crow::response res(buffer.str());

        if (path.size() >= 4 && path.substr(path.size()-4) == ".css") res.add_header("Content-Type", "text/css");
        else if (path.size() >= 3 && path.substr(path.size()-3) == ".js") res.add_header("Content-Type", "application/javascript");
        else if (path.size() >= 4 && path.substr(path.size()-4) == ".png") res.add_header("Content-Type", "image/png");
        else if (path.size() >= 4 && (path.substr(path.size()-4) == ".jpg" || path.substr(path.size()-5) == ".jpeg")) res.add_header("Content-Type", "image/jpeg");
        else if (path.size() >= 5 && path.substr(path.size()-5) == ".html") res.add_header("Content-Type", "text/html");
        else res.add_header("Content-Type", "application/octet-stream");

        return res;
    });



    CROW_ROUTE(app, "/add_product").methods(crow::HTTPMethod::POST)
    ([&products, &db_products](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);

        auto token = req.get_header_value("Authorization");
    if (token.empty() || sessions.find(token) == sessions.end())
        return crow::response(401, "Not logged in");
    string username = sessions[token];

        Product p;
        p.owner = username;
        p.code = body["code"].i();
        p.brand = body["brand"].s();
        p.name = body["name"].s();
        p.quantity = body["quantity"].i();
        p.stock_alert = body["stock_alert"].i();
        p.cost = body["cost"].d();
        p.price = body["price"].d();
        p.discount = body["discount"].d();

     for(auto& prod : products)
         if(prod.code == p.code && prod.owner == username)
             return crow::response(409, "Product exists");

         products.push_back(p);
         saveProducts(db_products, products);
        
         return crow::response(200, "Product added");
        
    });


    CROW_ROUTE(app, "/update_product").methods(crow::HTTPMethod::POST)
    ([&products, &db_products](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);

        auto token = req.get_header_value("Authorization");
    if (token.empty() || sessions.find(token) == sessions.end())
        return crow::response(401, "Not logged in");
    string username = sessions[token];

        if(updateProduct(body, products, db_products, username) )
            return crow::response(200, "Updated");
        else
            return crow::response(404, "Product not found");
    });


    CROW_ROUTE(app, "/delete_product").methods(crow::HTTPMethod::POST)
    ([&products, &db_products](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);

        auto token = req.get_header_value("Authorization");
    if (token.empty() || sessions.find(token) == sessions.end())
        return crow::response(401, "Not logged in");
    string username = sessions[token];

        int code = body["code"].i();
        if(deleteProduct(code, products, db_products, username) )
            return crow::response(200, "Deleted");
        else
            return crow::response(404, "Product not found");
    });



   CROW_ROUTE(app, "/add_sale").methods(crow::HTTPMethod::POST)
   ([&db_sales](const crow::request& req) {

    auto body = crow::json::load(req.body);
    if (!body) return crow::response(400);

    auto token = req.get_header_value("Authorization");
    if (token.empty() || sessions.find(token) == sessions.end())
        return crow::response(401, "Not logged in");

   string username = sessions[token];
transform(username.begin(), username.end(), username.begin(), ::tolower);



    sale s;
    s.owner = username;
    s.code = body["code"].i();
    s.brand = body["brand"].s();
    s.name = body["name"].s();
    s.quantity = body["quantity"].i();
    s.price = body["price"].d();
    s.discount = body["discount"].d();
    s.total_price = s.price * s.quantity;
    s.date = getCurrentDateTime();

    // Local vector just for this request
    vector<sale> sales{s};
    saveSales(db_sales, sales);

    return crow::response(200, "Sale added");
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

    return 0;
}



// --- IGNORE --- 

//  Compile command:

/* cd source_code
clang++ product_manager.cpp \
-std=c++17 \
-Iexternal/crow/include \
-Iexternal/asio/asio/include \
-I/opt/homebrew/opt/openssl@3/include \
-L/opt/homebrew/opt/openssl@3/lib \
-lsqlite3 -lcrypto -lpthread \
-o product_manager_app
./product_manager_app

*/

//  Access the application at:
// http://localhost:18080/
