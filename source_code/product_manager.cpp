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
    double price_discount;
    int vat_amount;
    double total_price;
};

struct sale {
    string owner;
    int customer_id;
    string customer;
    int code;
    string brand;
    string name;
    int quantity;
    double price;
    double discount;
    int vat_amount;
    double total_price;
    double sum_price;
    double cost;
    double total_cost;
    string date;
};

struct SalesReport {
    string month;
    int monthly_sales;
    int monthly_units;
    double monthly_revenue;
    double monthly_cost;
    int vat_amount;
    double monthly_profit;
};

struct UsersData {
    string owner;
    string username;
    string role;
};

struct CustomerData{
    string owner;
    int customer_id;
    string customer;
     string type;
    string email;
    int phone;
    string address;
    int postCode;
    
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
    const char* sql = "SELECT code, brand, name, quantity, stock_alert, cost, price, discount, vat_amount, price_discount, total_price FROM products WHERE owner = ?;";
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
        p.vat_amount = sqlite3_column_int(stmt, 8);
        p.price_discount = sqlite3_column_double(stmt, 9);
        p.total_price = sqlite3_column_double(stmt, 10);
        products.push_back(p);
    }
    sqlite3_finalize(stmt);
}

void saveProducts(sqlite3* db, const vector<Product>& products) {
    sqlite3_exec(db, "DELETE FROM products;", nullptr, nullptr, nullptr);
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO products (owner, code, brand, name, quantity, stock_alert, cost, price, discount,  vat_amount, price_discount, total_price) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
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
         sqlite3_bind_double(stmt, 10, p.vat_amount);
        sqlite3_bind_double(stmt, 11, p.price_discount);
        sqlite3_bind_double(stmt, 12, p.total_price);

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
            p.vat_amount = body["vat_amount"].i();
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

    const char* sql =
       "SELECT s.customer_id, c.customer, s.code, s.brand, s.name, s.quantity, s.price, s.discount, s.vat_amount, s.total_price, s.sum_price, s.cost, s.total_cost, s.date "
        "FROM sales s "
        "LEFT JOIN customers c ON s.customer_id = c.customer_id "
        "WHERE LOWER(s.owner) = LOWER(?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Failed to prepare loadSales stmt: " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

     while (sqlite3_step(stmt) == SQLITE_ROW) {
    sale s;
    s.owner = username;
    s.customer_id = sqlite3_column_int(stmt, 0);
    const unsigned char* customer_text = sqlite3_column_text(stmt, 1);
    s.customer = customer_text ? reinterpret_cast<const char*>(customer_text) : "";

    s.code = sqlite3_column_int(stmt, 2);
    const unsigned char* brand_text = sqlite3_column_text(stmt, 3);
    s.brand = brand_text ? reinterpret_cast<const char*>(brand_text) : "";

    const unsigned char* name_text = sqlite3_column_text(stmt, 4);
    s.name = name_text ? reinterpret_cast<const char*>(name_text) : "";

    s.quantity = sqlite3_column_int(stmt, 5);
    s.price = sqlite3_column_double(stmt, 6);
    s.discount = sqlite3_column_double(stmt, 7);
    s.vat_amount = sqlite3_column_double(stmt, 8);
    s.total_price = sqlite3_column_double(stmt, 9);
    s.sum_price = sqlite3_column_double(stmt, 10);
    s.cost = sqlite3_column_double(stmt, 11);
    s.total_cost = sqlite3_column_double(stmt, 12);

    const unsigned char* date_text = sqlite3_column_text(stmt, 13);
    s.date = date_text ? reinterpret_cast<const char*>(date_text) : "";

    sales.push_back(s);
}

    sqlite3_finalize(stmt);
}



void saveSales(sqlite3* db, const vector<sale>& sales) {
    sqlite3_stmt* stmt;

    const char* sql =
        "INSERT INTO sales (owner, customer_id, code, brand, name, quantity, "
        "price, discount, vat_amount, total_price, sum_price, cost, total_cost, date) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Failed to prepare saveSales stmt: " << sqlite3_errmsg(db) << endl;
        return;
    }

    for (const auto& s : sales) {
        sqlite3_bind_text(stmt, 1, s.owner.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, s.customer_id);
        sqlite3_bind_int(stmt, 3, s.code);
        sqlite3_bind_text(stmt, 4, s.brand.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, s.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 6, s.quantity);
        sqlite3_bind_double(stmt, 7, s.price);
        sqlite3_bind_double(stmt, 8, s.discount);
        sqlite3_bind_double(stmt, 9, s.vat_amount);
        sqlite3_bind_double(stmt, 10, s.total_price);
        sqlite3_bind_double(stmt, 11, s.sum_price);
        sqlite3_bind_double(stmt, 12, s.cost);
        sqlite3_bind_double(stmt, 13, s.total_cost);

        string date_to_save = s.date.empty() ? getCurrentDateTime() : s.date;
        sqlite3_bind_text(stmt, 14, date_to_save.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "Insert failed: " << sqlite3_errmsg(db) << endl;
        }

        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
    }

    sqlite3_finalize(stmt);
}


void salesReportGen(sqlite3* db, vector<SalesReport>& reports, const string& username, int month, int year) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT COUNT(code) AS total_sales, SUM(quantity) AS total_units, SUM(sum_price) AS total_revenue, COUNT(*) AS sale_count, SUM(total_cost) AS total_cost, vat_amount FROM sales "
                      "WHERE LOWER(owner) = LOWER(?) AND strftime('%m', date) = ? AND strftime('%Y', date) = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr <<" Failed to prepare SalesReport stmt: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    std::ostringstream mm;
    mm << std::setw(2) << std::setfill('0') << month;
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, mm.str().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, std::to_string(year).c_str(), -1, SQLITE_TRANSIENT);
    cout << "Binding month: " << mm.str() << ", year: " << year << endl;
    cout << "SQL bind: username=" << username
     << ", month=" << mm.str()
     << ", year=" << year << endl;


    

    if (sqlite3_step(stmt) == SQLITE_ROW) {
    SalesReport r;
    r.monthly_sales = sqlite3_column_int(stmt, 0);
    r.monthly_units = sqlite3_column_int(stmt, 1);
    r.monthly_revenue = sqlite3_column_double(stmt, 2);
    r.monthly_cost = sqlite3_column_double(stmt, 4);
    r.vat_amount = sqlite3_column_int(stmt, 5);
    r.monthly_profit = r.monthly_revenue - r.monthly_cost;
    r.month = std::to_string(month) + "/" + std::to_string(year);
    reports.push_back(r);
    }



    sqlite3_finalize(stmt);
}

void loadUsers(sqlite3* db, vector<UsersData>& Users, const string& username){
    sqlite3_stmt* stmt;
    const char* sql = "SELECT username, role FROM users "
                      "WHERE LOWER(owner) = LOWER(?);";

    if(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK){
        std::cerr << "Failed to prepare loadUsers stmt: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    while(sqlite3_step(stmt) == SQLITE_ROW){
        UsersData u;
        u.owner = username;
        const unsigned char* textVal;

        textVal = sqlite3_column_text(stmt, 0);
        u.username = textVal ? reinterpret_cast<const char*>(textVal) : "";

        textVal = sqlite3_column_text(stmt, 1);
        u.role = textVal ? reinterpret_cast<const char*>(textVal) : "";

        Users.push_back(u);
    }

    sqlite3_finalize(stmt);
}


void loadCustomers(sqlite3* db, vector<CustomerData>& Customers, const string& username){
    sqlite3_stmt* stmt;
    const char* sql = "SELECT customer_id, customer, type, email, phone, address, postCode FROM customers "
                      "WHERE LOWER(owner) = LOWER(?);";

    if(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK){
        std::cerr << "Failed to prepare loadCustomers stmt: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    int colCount = sqlite3_column_count(stmt);
std::cout << "Columns returned: " << colCount << std::endl;


    while(sqlite3_step(stmt) == SQLITE_ROW){
        CustomerData c;
        c.owner = username;

        std::cout << "Customer ID: " << sqlite3_column_int(stmt, 0) << std::endl;


        const unsigned char* textVal;

        c.customer_id = sqlite3_column_int(stmt, 0);

        textVal = sqlite3_column_text(stmt, 1);   
        c.customer = textVal ? reinterpret_cast<const char*>(textVal) : "";

        textVal = sqlite3_column_text(stmt, 2);   
        c.type = textVal ? reinterpret_cast<const char*>(textVal) : "";

        textVal = sqlite3_column_text(stmt, 3);  
        c.email = textVal ? reinterpret_cast<const char*>(textVal) : "";

        textVal = sqlite3_column_text(stmt, 5); 
        c.address = textVal ? reinterpret_cast<const char*>(textVal) : "";

        c.phone = sqlite3_column_int(stmt, 4);   
        c.postCode = sqlite3_column_int(stmt, 6); 
        Customers.push_back(c);
    }

    sqlite3_finalize(stmt);
}


void saveCustomers(sqlite3* db, std::vector<CustomerData>& Customers, const string& username) {

    const char* sql ="INSERT INTO customers (owner, customer, type, email, phone, address, postCode) "
                     "VALUES (?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare saveCustomers stmt: "
                  << sqlite3_errmsg(db) << std::endl;
        return;
    }

    for (const auto& c : Customers) {
        sqlite3_bind_text(stmt, 1, c.owner.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, c.customer.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, c.type.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, c.email.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int (stmt, 5, c.phone);
        sqlite3_bind_text(stmt, 6, c.address.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int (stmt, 7, c.postCode);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Update failed: "
                      << sqlite3_errmsg(db) << std::endl;
        }

        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
    }

    sqlite3_finalize(stmt);
}

bool updateCustomer(const crow::json::rvalue& body, sqlite3* db, const std::string& username) {
    if (!body.has("customer_id"))
        return false;

    int customer_id = body["customer_id"].i();

    const char* sql =
        "UPDATE customers SET customer=?, type=?, email=?, phone=?, address=?, postCode=? "
        "WHERE customer_id=? AND owner=?;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;
        sqlite3_bind_text(stmt, 1, std::string(body["customer"].s()).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, std::string(body["type"].s()).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, std::string(body["email"].s()).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, std::string(body["phone"].s()).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, std::string(body["address"].s()).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, std::string(body["postCode"].s()).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 7, body["customer_id"].i());
    sqlite3_bind_text(stmt, 8, username.c_str(), -1, SQLITE_TRANSIENT);

    std::cout << "Updating ID: " << customer_id << std::endl;
    std::cout << "Owner from token: '" << username << "'" << std::endl;





    bool updated = (sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0);
    sqlite3_finalize(stmt);

    int changes = sqlite3_changes(db);
if (changes == 0) {
    std::cerr << "No row updated. customer_id=" << customer_id << ", owner=" << username << std::endl;
    return false;
}


    return updated;
}







int main() {
    // --- DATABASES ---
    sqlite3* db_prodexa;

    if (sqlite3_open("databases/prodexa.db", &db_prodexa) != SQLITE_OK) {
        cerr << "Failed to open prodexa.db\n";
        return 1;
    }
    cout << "SALES DB FILE: "
     << sqlite3_db_filename(db_prodexa, "main")
     << endl;



   // string username = sessions[token];

   // vector<sale> sales;

    vector<Product> products;
    vector<CustomerData> Customers;


    crow::SimpleApp app;


    // --- LOGIN API ---
    CROW_ROUTE(app, "/login").methods(crow::HTTPMethod::POST)([&db_prodexa](const crow::request& req){
    auto body = crow::json::load(req.body);
    if (!body) return crow::response(400);

    std::string username = body["username"].s();
    std::string password = body["password"].s();
    std::string input_hash = hashPassword(password);

    sqlite3_stmt* stmt;
    const char* sql = "SELECT password_hash, role FROM users WHERE username = ?;";
    sqlite3_prepare_v2(db_prodexa, sql, -1, &stmt, nullptr);
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
CROW_ROUTE(app, "/add_user").methods(crow::HTTPMethod::POST)([&db_prodexa](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);
        
        string username = body["username"].s();
        string owner  = username;
        string password = body["password"].s();
        string role = body["role"].s();
        bool termsAccepted = body["termsAccepted"].b();
        string hashed = hashPassword(password);
        time_t terms_accepted_at = time(nullptr);

         if (!termsAccepted) {
            return crow::response(400, "Terms must be accepted");
        }
        sqlite3_stmt* stmt;
        const char* sql = "INSERT INTO users (owner, username, password_hash, role, termsAccepted, terms_accepted_at) VALUES (?, ?, ?, ?, ?, ?);";
        if (sqlite3_prepare_v2(db_prodexa, sql, -1, &stmt, nullptr) != SQLITE_OK)
            return crow::response(500);

        sqlite3_bind_text(stmt, 1, owner.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, hashed.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, role.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 5, termsAccepted ? 1 : 0);
        sqlite3_bind_int64(stmt, 6, static_cast<sqlite3_int64>(terms_accepted_at));

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
CROW_ROUTE(app, "/current_user")([&db_prodexa](const crow::request& req){
    auto token = req.get_header_value("Authorization");
    if (token.empty() || sessions.find(token) == sessions.end())
        return crow::response(401, "Not logged in");

    std::string username = sessions[token];

    sqlite3_stmt* stmt;
    const char* sql = "SELECT role FROM users WHERE username = ?;";
    sqlite3_prepare_v2(db_prodexa, sql, -1, &stmt, nullptr);
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
([&products, &db_prodexa](const crow::request& req){
    auto token = req.get_header_value("Authorization");
if (token.empty() || sessions.find(token) == sessions.end())
    return crow::response(401, "Not logged in");

string username = sessions[token];

loadProducts(db_prodexa, products, username);


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
        obj["price_discount"] = p.price - (p.price * p.discount / 100.0);
        obj["vat_amount"] = p.vat_amount;
        obj["total_price"] =  (p.price * (1 - p.discount / 100.0)) * (1 + p.vat_amount / 100.0);
        out["products"][i++] = std::move(obj);

    }

    return crow::response(200, out);
});

    // --- GET PRODUCT API ---
CROW_ROUTE(app, "/get_product")
([&products, &db_prodexa](const crow::request& req){
    auto query_param = req.url_params.get("query");
    if (!query_param) return crow::response(400, "Missing query");

    string query = query_param;

    auto token = req.get_header_value("Authorization");
if (token.empty() || sessions.find(token) == sessions.end())
    return crow::response(401, "Not logged in");

string username = sessions[token];

loadProducts(db_prodexa, products, username);


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
            res["vat_amount"] = p.vat_amount;
            res["total_price"] = p.total_price;
            return crow::response(200, res);
        }
    }

    return crow::response(404, "Product not found");
});

// --- Sales Data API ---
CROW_ROUTE(app, "/api/sales")
([ &db_prodexa](const crow::request& req){
    auto token = req.get_header_value("Authorization");
if (token.empty() || sessions.find(token) == sessions.end())
    return crow::response(401, "Not logged in");

string username = sessions[token];
transform(username.begin(), username.end(), username.begin(), ::tolower);

vector<sale> sales;
 loadSales(db_prodexa, sales, username);
 cout << "Sales loaded: " << sales.size() << endl;
for (auto& s : sales) {
    cout << s.code << " " << s.name << " " << s.total_price << " " << s.date << endl;
} 
cout << "SESSION USERNAME = [" << username << "]" << endl;


    crow::json::wvalue out;
    out["sales"] = crow::json::wvalue::list();

    int i = 0;
    for (const auto& s : sales) {
        crow::json::wvalue obj;
        obj["user"] = s.owner;
        obj["customer"] = s.customer;
        obj["code"] = s.code;
        obj["brand"] = s.brand;
        obj["name"] = s.name;
        obj["quantity"] = s.quantity;
        obj["price"] = s.price;
        obj["discount"] = s.discount;
        obj["vat_amount"] = s.vat_amount;
        obj["total_price"] = (s.price * (1 - s.discount / 100.0)) * (1 + s.vat_amount / 100.0);
        obj["date"] = s.date;
        out["sales"][i++] = std::move(obj);
    }

    return crow::response(200, out);
});

CROW_ROUTE(app, "/api/sales_report")
([&db_prodexa](const crow::request& req){
    auto token = req.get_header_value("Authorization");
if (token.empty() || sessions.find(token) == sessions.end())
    return crow::response(401, "Not logged in");

     auto monthStr = req.url_params.get("month");
    auto yearStr  = req.url_params.get("year");

    if (!monthStr || !yearStr)
        return crow::response(400, "Missing month or year");

        int month, year;

        try{
            month = std::stoi(monthStr);
            year = std::stoi(yearStr);
        } catch (const std::exception& e) {
            return crow::response(400, "Invalid month or year format");
        }

         if (month < 1 || month > 12)
            return crow::response(400, "Month must be between 1 and 12");

        if(year < 2000 || year > 2100)
            return crow::response(400, "Year must be between 2000 and 2100");
    

string username = sessions[token];
transform(username.begin(), username.end(), username.begin(), ::tolower);

vector<SalesReport> reports;
 salesReportGen(db_prodexa, reports, username, month, year);
cout << "Reports generated: " << reports.size() << endl;
    crow::json::wvalue out;
    out["reports"] = crow::json::wvalue::list();

    int i = 0;
    for (const auto& r : reports) {
        crow::json::wvalue obj;
        obj["month"] = r.month;
        obj["monthly_sales"] = r.monthly_sales;
        obj["monthly_units"] = r.monthly_units;
        obj["monthly_revenue"] = r.monthly_revenue;
        obj["monthly_cost"] = r.monthly_cost;
        obj["vat_amount"] = r.vat_amount;
        obj["monthly_profit"] = r.monthly_profit;
        out["reports"][i++] = std::move(obj);
    }

    return crow::response(200, out);
});


CROW_ROUTE(app, "/api/users")
([&db_prodexa](const crow::request& req){
    auto token = req.get_header_value("Authorization");
    if(token.empty() || sessions.find(token) == sessions.end())
        return crow::response(401, "Not logged in");

    string username = sessions[token];

    vector<UsersData> Users;
    loadUsers(db_prodexa, Users, username);

    crow::json::wvalue out;
    out["Users"] = crow::json::wvalue::list(); // initialize as list

    // Build a list of wvalue
    std::vector<crow::json::wvalue> tempList;
    for (const auto& u : Users) {
        crow::json::wvalue obj;
        obj["username"] = u.username;
        obj["role"] = u.role;
        obj["owner"] = u.owner;
        tempList.push_back(std::move(obj));
    }

    out["Users"] = std::move(tempList); // assign the entire vector at once

    return crow::response(200, out);
});


// --- DISPLAY CUSTOMERS ---
CROW_ROUTE(app, "/api/customers")
([&db_prodexa](const crow::request& req){
    auto token = req.get_header_value("Authorization");
if(token.empty() || sessions.find(token) == sessions.end())
   return crow::response(401,"Not logged in");

   string username = sessions[token];


   vector<CustomerData> Customers;
   Customers.clear();

   loadCustomers(db_prodexa, Customers, username);

   crow::json::wvalue out;
   out["Customers"] = crow::json::wvalue::list();

  std::vector<crow::json::wvalue> tempCustomers;
for (const auto& c : Customers) {
    crow::json::wvalue obj;
    obj["customer_id"] = c.customer_id;
    obj["customer"] = c.customer;
    obj["type"] = c.type;
    obj["email"] = c.email;
    obj["phone"] = c.phone;
    obj["address"] = c.address;
    obj["postCode"] = c.postCode;
    tempCustomers.push_back(std::move(obj));
}
out["Customers"] = std::move(tempCustomers);


   return crow::response(200, out);

});


// --- GET CUSTOMER INFO ---
CROW_ROUTE(app, "/get_customers")
([&Customers, &db_prodexa](const crow::request& req){
    auto query_param = req.url_params.get("query");
    if (!query_param) return crow::response(400, "Missing query");

    string query = query_param;

    auto token = req.get_header_value("Authorization");
if (token.empty() || sessions.find(token) == sessions.end())
    return crow::response(401, "Not logged in");

string username = sessions[token];
vector<CustomerData> Customers;
Customers.clear();

loadCustomers(db_prodexa, Customers, username);


    for (const auto& c : Customers) {
       string name_lower = c.customer;
       string query_lower = query;

    transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
    transform(query_lower.begin(), query_lower.end(), query_lower.begin(), ::tolower);

if (name_lower.find(query_lower) != string::npos) {
            crow::json::wvalue res;
            res["customer_id"] = c.customer_id;
            res["customer"] = c.customer;
            res["type"] = c.type;
            res["email"] = c.email;
            res["phone"] = c.phone;
            res["address"] = c.address;
            res["postCode"] = c.postCode;
            return crow::response(200, res);
        }
    }

    return crow::response(404, "Customer not found");
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

// --- SALES REPORT PAGE ---
CROW_ROUTE(app, "/sales_report")([](){
    ifstream file("static/html/sales_report.html", ios::binary);
    if (!file.is_open())
        return crow::response(404, "sales_report.html not found");

    stringstream buffer;
    buffer << file.rdbuf();
    crow::response res(buffer.str());
    res.add_header("Content-Type", "text/html");
    return res;

});

// --- USERS PAGE --- 

// --- VIEW USERS PAGE ---
CROW_ROUTE(app, "/view_users")([](){
    ifstream file("static/html/view_users.html", ios::binary);
    if(!file.is_open())
    return crow::response(404,"view_users.html not found");

    stringstream buffer;
    buffer << file.rdbuf();
    crow::response res(buffer.str());
    res.add_header("Content-Type", "text/html");
    return res;

});

// --- CUSTOMERS PAGE ---

// --- VIEW CUSTOMERS PAGE ---
CROW_ROUTE(app, "/view_customers")([](){
    ifstream file("static/html/view_customers.html", ios::binary);
    if(!file.is_open())
    return crow::response(404,"view_customers.html not found");

    stringstream buffer;
    buffer << file.rdbuf();
    crow::response res(buffer.str());
    res.add_header("Content-Type", "text/html");
    return res;

});


// --- ADD CUSTOMER PAGE ---
CROW_ROUTE(app, "/add_customers")([](){
    ifstream file("static/html/add_customers.html", ios::binary);
    if(!file.is_open())
    return crow::response(404,"add_customers.html not found");

    stringstream buffer;
    buffer << file.rdbuf();
    crow::response res(buffer.str());
    res.add_header("Content-Type", "text/html");
    return res;

});

// --- EDIT CUSTOMERS PAGE ---
CROW_ROUTE(app, "/edit_customers")([](){
    ifstream file("static/html/edit_customers.html", ios::binary);
    if(!file.is_open())
    return crow::response(404,"edit_customers.html not found");

    stringstream buffer;
    buffer << file.rdbuf();
    crow::response res(buffer.str());
    res.add_header("Content-Type", "text/html");
    return res;

});


// --- MENU BAR ---
CROW_ROUTE(app, "/menu_bar")([](){
    ifstream file("static/html/menu_bar.html", ios::binary);
    if(!file.is_open())
    return crow::response(404,"menu_bar.html not found");

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
    ([&products, &db_prodexa](const crow::request& req){
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
        p.vat_amount = body["vat_amount"].d();

     for(auto& prod : products)
         if(prod.code == p.code && prod.owner == username)
             return crow::response(409, "Product exists");

        if(p.quantity < 0 || p.stock_alert < 0 || p.cost < 0 || p.price < 0 || p.discount < 0 || p.vat_amount < 0)
            return crow::response(400, "Negative values are not allowed");

         products.push_back(p);
         saveProducts(db_prodexa, products);
        
         return crow::response(200, "Product added");
        
    });


    CROW_ROUTE(app, "/update_product").methods(crow::HTTPMethod::POST)
    ([&products, &db_prodexa](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);

        auto token = req.get_header_value("Authorization");
    if (token.empty() || sessions.find(token) == sessions.end())
        return crow::response(401, "Not logged in");
    string username = sessions[token];

        if(updateProduct(body, products, db_prodexa, username) )
            return crow::response(200, "Updated");
        else
            return crow::response(404, "Product not found");
    });


    CROW_ROUTE(app, "/delete_product").methods(crow::HTTPMethod::POST)
    ([&products, &db_prodexa](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);

        auto token = req.get_header_value("Authorization");
    if (token.empty() || sessions.find(token) == sessions.end())
        return crow::response(401, "Not logged in");
    string username = sessions[token];

        int code = body["code"].i();
        if(deleteProduct(code, products, db_prodexa, username) )
            return crow::response(200, "Deleted");
        else
            return crow::response(404, "Product not found");
    });



   CROW_ROUTE(app, "/add_sale").methods(crow::HTTPMethod::POST)
   ([&db_prodexa,&products](const crow::request& req) {

    auto body = crow::json::load(req.body);
    if (!body) return crow::response(400);

    auto token = req.get_header_value("Authorization");
    if (token.empty() || sessions.find(token) == sessions.end())
        return crow::response(401, "Not logged in");

   string username = sessions[token];
transform(username.begin(), username.end(), username.begin(), ::tolower);



    sale s;
    s.owner = username;
    s.customer_id = body["customer_id"].i();
    s.code = body["code"].i();
    s.brand = body["brand"].s();
    s.name = body["name"].s();
    s.quantity = body["quantity"].i();
    s.price = body["price"].d();
    s.discount = body["discount"].d();
    s.vat_amount = body["vat_amount"].d();
    s.total_price = (s.price * (1 - s.discount / 100.0)) * (1 + s.vat_amount / 100.0);
    s.sum_price = s.total_price * s.quantity;
    s.cost = body["cost"].d();
    s.total_cost = s.cost * s.quantity;
    s.date = getCurrentDateTime();

    // Local vector just for this request
    vector<sale> sales;
    std::cout << "Incoming customer_id: " 
          << body["customer_id"].i() 
          << std::endl;


    // Update product stock
    loadProducts(db_prodexa, products, username);
    for (auto& p : products) {
        if (p.code == s.code && p.owner == username) {
            p.quantity -= s.quantity;
            if(p.quantity < 0) {
                p.quantity = p.quantity + s.quantity; // revert stock change
                return crow::response(400, "Insufficient stock");
            }
            if(p.quantity < p.stock_alert) {
                sales.push_back(s);
                saveProducts(db_prodexa, products);
                saveSales(db_prodexa, sales);
                std::cout << "Sale struct customer_id: " 
          << s.customer_id 
          << std::endl;

               return crow::response(403, "Sale added. Warning: Stock below alert level!");

            }
            else {
                 sales.push_back(s);
                saveProducts(db_prodexa, products);
                saveSales(db_prodexa, sales);
                std::cout << "Sale struct customer_id: " 
          << s.customer_id 
          << std::endl;

                return crow::response(200, "Sale added successfully");
            }
        }
    }
   return crow::response(404, "Product not found");
});

// --- ADD CUSTOMER ---
CROW_ROUTE(app, "/add_customer").methods(crow::HTTPMethod::POST)
([&Customers, &db_prodexa](const crow::request& req){

    auto body = crow::json::load(req.body);
    if (!body) return crow::response(400);

    auto token = req.get_header_value("Authorization");
    if (token.empty() || sessions.find(token) == sessions.end())
        return crow::response(401, "Not logged in");

    string username = sessions[token];

    CustomerData c;
    c.owner    = username;
    c.customer = body["customer"].s();
    c.type     = body["type"].s();
    c.email    = body["email"].s();
    c.phone    = body["phone"].i();
    c.address  = body["address"].s();
    c.postCode = body["postCode"].i();

    for (const auto& Cust : Customers) {
        if (Cust.email == c.email &&
            Cust.customer == c.customer &&
            Cust.owner == username) {
            return crow::response(409, "Customer exists");
        }
    }

    Customers.push_back(c);
    saveCustomers(db_prodexa, Customers, username);

    return crow::response(200, "Customer added");
});


// --- UPDATE CUSTOMER ---
CROW_ROUTE(app, "/update_customer").methods(crow::HTTPMethod::POST)
([&db_prodexa](const crow::request& req){
    auto body = crow::json::load(req.body);
    if (!body) return crow::response(400);

    auto token = req.get_header_value("Authorization");
    if (token.empty() || sessions.find(token) == sessions.end())
        return crow::response(401, "Not logged in");

    std::string username = sessions[token];

    if (updateCustomer(body, db_prodexa, username))
        return crow::response(200, "Customer updated successfully");
    else if(username != body["owner"].s())
        return crow::response(403, "Forbidden: You do not have permission to perform this action");
    else
    return crow::response(404, "Customer not found");
});






    // --- RUN SERVER ---
    int port = 18080;
    if (const char* env_p = std::getenv("PORT")) {
        port = std::stoi(env_p);
    }
    app.port(port).multithreaded().run();

    // --- CLOSE DATABASES ---
    sqlite3_close(db_prodexa);

    return 0;
}



// --- IGNORE --- 

//  Compile command:

/*   cd source_code
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
