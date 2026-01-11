#include<iostream>
#include<string>
#include<fstream>
#include<iomanip>
#include<cmath>
#include<ctime>
#include <sqlite3.h>
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


 //loads all products from file
   void loadProducts(sqlite3* db, vector<Product>& products) {
    sqlite3_stmt* stmt;

    const char* sql =
        "SELECT code, brand, name, quantity, stock_alert, cost, price, discount FROM products;";

    products.clear();

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

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


//saves all products to file
    void saveProducts(sqlite3* db, const vector<Product>& products) {
    sqlite3_stmt* stmt;

    const char* sql =
        "INSERT OR REPLACE INTO products "
        "(code, brand, name, quantity, stock_alert, cost, price, discount) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

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




//lets user add new product
   void addProduct(vector<Product>& products) {
    Product p;

    cout << "<--- ADD NEW PRODUCT --->\n";
    cout << "Enter product code:\n";
    cin >> p.code;

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Enter product brand:\n";
    getline(cin, p.brand);

    cout << "Enter product name:\n";
    getline(cin, p.name);

    cout << "Enter product quantity:\n";
    cin >> p.quantity;

    cout << "Enter product stock alert:\n";
    cin >> p.stock_alert;

    cout << "Enter product cost:\n";
    cin >> p.cost;

    cout << "Enter product price:\n";
    cin >> p.price;

    cout << "Enter product discount:\n";
    cin >> p.discount;

    products.push_back(p);
}

//lets user delete product
    void deleteProduct(sqlite3* db, vector<Product>& products) {
    int target;
    cout << "<--- DELETE PRODUCT --->\n";
    cout<<"Enter product code to delete:\n";
    cin >> target;

    products.erase(
        remove_if(products.begin(), products.end(),
                  [&](const Product& p) { return p.code == target; }),
        products.end());

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "DELETE FROM products WHERE code = ?;", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, target);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

//lets user update an already existing product
    void updateProduct(vector<Product>& products) {
    int target;
    cout << "<--- UPDATE PRODUCT --->\n";
    cout<<"Enter product code to update:\n";
    cin >> target;

    for (auto& p : products) {
        if (p.code == target) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Enter new brand:\n";
            getline(cin, p.brand);
            cout << "Enter new name:\n";
            getline(cin, p.name);
            cout<<"Enter new quantity:\n";
             cin >> p.quantity;
            cout<<"Enter new stock alert:\n";
            cin >> p.stock_alert;
            cout<<"Enter new cost:\n";
            cin >> p.cost;
            cout<<"Enter new price:\n";
            cin >> p.price;
            cout<<"Enter new discount:\n";
            cin >> p.discount;
            return;
        }
    }
}

//lets user sell a product and records it to a sales audit 
   void sellProduct(sqlite3* db, vector<Product>& products, string user, int& total_sales) {
    int target, qty;
    cout << "<--- SELL PRODUCT --->\n";
    cout << "Enter product code to sell:\n";
    cin >> target;
    cout<<"Enter quantity to sell:\n";
    cin >> qty;

    for (auto& p : products) {
        if (p.code == target) {
            if (qty > p.quantity) {
                cout << "Not enough stock\n";
                return;
            }

            p.quantity -= qty;
            total_sales += qty;

            sqlite3_stmt* stmt;
            sqlite3_prepare_v2(db, "INSERT INTO sales (user, code, brand, quantity, price, discount,total_value) VALUES (?, ?, ?, ?, ?, ?, ?);", -1, &stmt, nullptr);
            sqlite3_bind_text(stmt, 1, user.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 2, target);
            sqlite3_bind_text(stmt, 3, p.brand.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 4, qty);
            sqlite3_bind_double(stmt, 5, p.price);
            sqlite3_bind_double(stmt, 6, p.discount);
            double total_value = (p.discount != 0)
                ? (qty * p.price) - (p.discount * qty)
                : (qty * p.price);
            sqlite3_bind_double(stmt, 7, total_value);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);

            cout << "Sold " << qty << " of " << p.name << "\n";
            return;
        }
    }
}


//calculates total products sold so far
void SumSold(sqlite3* db,vector<Product>& products, string current_user_name, int total_sales, int &sum_sold){
    sum_sold = total_sales;
    cout<<"Total products sold so far:"<<sum_sold<<endl;
}

// lets user search for a product by its code
void sequentialSearchByCode(const vector<Product>& products) {
    int target;
    bool found = false;

    cout << "Enter product code:\n";
    cin >> target;

    for (const auto& p : products) {
        if (p.code == target) {
            double total_cost = p.quantity * p.cost;
            double value = (p.discount != 0)
                ? (p.quantity * p.price) - (p.discount * p.quantity)
                : (p.quantity * p.price);

            cout << left
                 << setw(8) << "CODE"
                 << setw(15) << "BRAND"
                 << setw(18) << "NAME"
                 << setw(10) << "QUANTITY"
                 << setw(13) << "STOCK ALERT"
                 << setw(12) << "COST"
                 << setw(12) << "PRICE"
                 << setw(12) << "DISCOUNT"
                 << setw(18) << "TOTAL COST"
                 << setw(18) << "TOTAL VALUE" << endl;

            cout << setw(8) << p.code
                 << setw(15) << p.brand
                 << setw(18) << p.name
                 << setw(10) << p.quantity
                 << setw(13) << p.stock_alert
                 << setw(12) << fixed << setprecision(2) << p.cost
                 << setw(12) << p.price
                 << setw(12) << p.discount
                 << setw(18) << total_cost
                 << setw(18) << value << endl;

            if (p.quantity <= p.stock_alert) {
                cout << "WARNING: stock below alert level\n";
            }

            found = true;
            break;
        }
    }

    if (!found) {
        cout << "No product found\n";
    }
}


// lets user search for a product by its brand
void sequentialSearchByBrand(const vector<Product>& products) {
    string target;
    bool found = false;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Enter product brand:\n";
    getline(cin, target);

    for (const auto& p : products) {
        if (p.brand == target) {
            double total_cost = p.quantity * p.cost;
            double value = (p.discount != 0)
                ? (p.quantity * p.price) - (p.discount * p.quantity)
                : (p.quantity * p.price);

            cout << left
                 << setw(8) << "CODE"
                 << setw(15) << "BRAND"
                 << setw(18) << "NAME"
                 << setw(10) << "QUANTITY"
                 << setw(13) << "STOCK ALERT"
                 << setw(12) << "COST"
                 << setw(12) << "PRICE"
                 << setw(12) << "DISCOUNT"
                 << setw(18) << "TOTAL COST"
                 << setw(18) << "TOTAL VALUE" << endl;

            cout << setw(8) << p.code
                 << setw(15) << p.brand
                 << setw(18) << p.name
                 << setw(10) << p.quantity
                 << setw(13) << p.stock_alert
                 << setw(12) << fixed << setprecision(2) << p.cost
                 << setw(12) << fixed << setprecision(2) << p.price
                 << setw(12) << fixed << setprecision(2) << p.discount
                 << setw(18) << fixed << setprecision(2) << total_cost
                 << setw(18) << fixed << setprecision(2) << value<< endl;

            if (p.quantity <= p.stock_alert) {
                cout<<"WARNING: stock below alert level\n";
            }

            found = true;
        }
    }

    if (!found){
        cout<<"No product found with brand: "<<target<<"\n";
    }
}

// lets user search for a product by its name
void sequentialSearchByName(const vector<Product>& products) {
    string target;
    bool found = false;

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Enter name:\n";
    getline(cin, target);

    for (const auto& p : products) {
        if (p.name == target) {
            double total_cost = p.quantity * p.cost;
            double value = (p.discount != 0)
                ? (p.quantity * p.price) - (p.discount * p.quantity)
                : (p.quantity * p.price);

            cout << setw(8) << p.code
                 << setw(15) << p.brand
                 << setw(18) << p.name
                 << setw(10) << p.quantity
                 << setw(13) << p.stock_alert
                 << setw(12) << p.cost
                 << setw(12) << p.price
                 << setw(12) << p.discount
                 << setw(18) << total_cost
                 << setw(18) << value << endl;

            found = true;
        }
    }

    if (!found) {
        cout << "No product found\n";
    }
}


// sorts all products using its code from smallest to biggest
void bubbleSort(vector<Product>& products) {
    for (size_t i = 0; i < products.size(); i++) {
        for (size_t j = 0; j < products.size() - i - 1; j++) {
            if (products[j].code > products[j + 1].code) {
                swap(products[j], products[j + 1]);
            }
        }
    }
}


//calculates the total sell value of all products 
void SumTotalValue(const vector<Product>& products, double& total_value) {
    total_value = 0.0;

    for (const auto& p : products) {
        total_value += (p.discount != 0)
            ? (p.quantity * p.price) - (p.discount * p.quantity)
            : (p.quantity * p.price);
    }
}


//lets user see all products and their information
void AllProducts(const vector<Product>& products) {
    double total_value = 0;

    cout << "<--- ALL PRODUCTS --->\n";

    for (const auto& p : products) {
        double total_cost = p.quantity * p.cost;
        double value = (p.discount != 0)
            ? (p.quantity * p.price) - (p.discount * p.quantity)
            : (p.quantity * p.price);

        total_value += value;
        
        cout<< left
             << setw(8) << "CODE"
             << setw(15) << "BRAND"
             << setw(18) << "NAME"
             << setw(10) << "QUANTITY"
             << setw(13) << "STOCK ALERT"
             << setw(12) << "COST"
             << setw(12) << "PRICE"
             << setw(12) << "DISCOUNT"
             << setw(18) << "TOTAL COST"
             << setw(18) << "TOTAL VALUE" << endl;

        cout << setw(8) << p.code
             << setw(15) << p.brand
             << setw(18) << p.name
             << setw(10) << p.quantity
             << setw(13) << p.stock_alert
             << setw(12) << p.cost
             << setw(12) << p.price
             << setw(12) << p.discount
             << setw(18) << total_cost
             << setw(18) << value << endl;

        if (p.quantity <= p.stock_alert) {
            cout << "WARNING: low stock\n";
        }
    }

    cout << "Total products: " << products.size() << endl;
    cout << "Total value: " << total_value << endl;
}


//gets user data from file
/*void LoadUsers(sqlite3* db, string username[],string password[], string role[], int &user_count){
    sqlite3_stmt* stmt;

    const char* sql =
        "SELECT username, password, role "
        "FROM users;";

    user_count = 0;

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        username[user_count] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        password[user_count] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        role[user_count] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user_count++;
    }
    sqlite3_finalize(stmt);
}*/

void AddUser(sqlite3* db) {
    string username, password, role;

    cout << "<--- ADD NEW USER --->" << endl;
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;

    do {
        cout << "Enter role (manager/cashier): ";
        cin >> role;
        if (role != "manager" && role != "cashier") {
            cout << "Invalid role. Try again.\n";
        }
    } while (role != "manager" && role != "cashier");

    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO users (username, password, role) VALUES (?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Failed to prepare statement for adding user.\n";
        return;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, role.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Error inserting user (maybe username already exists).\n";
    } else {
        cout << "User added successfully!\n";
    }

    sqlite3_finalize(stmt);
}


//requires user to login to use features depending on position
string UserLogin(sqlite3* db, string &current_user_name, string &current_user_pass, int &login_count) {
    string input_name, input_pass;

    cout << "<--- USER LOGIN --->" << endl;
    cout << "Enter username: ";
    cin >> input_name;
    cout << "Enter password: ";
    cin >> input_pass;

    sqlite3_stmt* stmt;
    const char* sql = "SELECT password, role FROM users WHERE username = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Failed to prepare login query\n";
        return "error";
    }

    sqlite3_bind_text(stmt, 1, input_name.c_str(), -1, SQLITE_TRANSIENT);

    string role = "error";

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        string db_pass = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        string db_role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        if (input_pass == db_pass) {
            current_user_name = input_name;
            current_user_pass = db_pass;
            role = db_role;
        } else {
            login_count++;
            cout << "Wrong username or password\n";
        }
    }

    sqlite3_finalize(stmt);
    return role;
}




//logs in file the time and user information when they entered 
void User_in_Audit(sqlite3* db,  string& current_user_name,  string& current_user_role) {
    sqlite3_stmt* stmt;

    time_t now = time(0);
    char dt[26]; // ctime_r safe version
    ctime_r(&now, dt);
    dt[strcspn(dt, "\n")] = 0; // remove newline

    const char* sql = "INSERT INTO audit (username, role, time) VALUES (?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Failed to prepare audit insert: " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, current_user_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, current_user_role.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, dt, -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Failed to insert audit record: " << sqlite3_errmsg(db) << endl;
    } else {
        cout << "Audit logged successfully.\n";
    }

    sqlite3_finalize(stmt);
}

void user_out_audit(sqlite3* db,  string& current_user_name,  string& current_user_role) {
    sqlite3_stmt* stmt;

    time_t now = time(0);
    char dt[26];
    ctime_r(&now, dt);
    dt[strcspn(dt, "\n")] = 0;

    const char* sql = "INSERT INTO audit (username, role, time) VALUES (?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Failed to prepare audit insert: " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, current_user_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, current_user_role.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, dt, -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Failed to insert audit record: " << sqlite3_errmsg(db) << endl;
    } else {
        cout << "Audit logged successfully.\n";
    }

    sqlite3_finalize(stmt);
}



//calls each function depending on user input
int main(){
     std::cout << "Product manager running!" << std::endl;
    vector<Product> products;

    string username[10];
    string password[10];
    string role[10];

    int user_count = 0;

    string current_user_name;
    string current_user_pass;
    string current_user_role;

    int login_count = 0;

    int action;
    int sub_action;

    double total_value;
    int total_sales = 0;
    int sum_sold = 0;

    sqlite3* db_products;
    sqlite3* db_users;
    sqlite3* db_audit;
    sqlite3* db_sales;


    int rc_sales = sqlite3_open("/Users/panayiotissavva/Documents/product_manager/source_code/sales.db", &db_sales); // must be a persistent file
if (rc_sales != SQLITE_OK) {
    cerr << "Cannot open database: " << sqlite3_errmsg(db_sales) << endl;
    sqlite3_close(db_sales);
    return 1;
} else {
    cout << "Database opened successfully at: sales.db" << endl;
}

    int rc_audit = sqlite3_open("/Users/panayiotissavva/Documents/product_manager/source_code/user_audit.db", &db_audit); // must be a persistent file
if (rc_audit != SQLITE_OK) {
    cerr << "Cannot open database: " << sqlite3_errmsg(db_audit) << endl;
    sqlite3_close(db_audit);
    return 1;
} else {
    cout << "Database opened successfully at: user_audit.db" << endl;
}

 
   int rc_pro = sqlite3_open("/Users/panayiotissavva/Documents/product_manager/source_code/products.db", &db_products); // must be a persistent file
if (rc_pro != SQLITE_OK) {
    cerr << "Cannot open database: " << sqlite3_errmsg(db_products) << endl;
    sqlite3_close(db_products);
    return 1;
} else {
    cout << "Database opened successfully at: products.db" << endl;
}


    int rc_user = sqlite3_open("/Users/panayiotissavva/Documents/product_manager/source_code/users.db", &db_users); // must be a persistent file
if (rc_user != SQLITE_OK) {
    cerr << "Cannot open database: " << sqlite3_errmsg(db_users) << endl;
    sqlite3_close(db_users);
    return 1;
} else {
    cout << "Database opened successfully at: users.db" << endl;
}



// Let user decide: login or add user
int choice;
cout << "1. Login\n2. Add User\nChoose an option: ";
cin >> choice;

if (choice == 2) {
    AddUser(db_users); // add user to database
    cout << "Now login with your credentials.\n";
}

current_user_role = UserLogin(db_users, current_user_name, current_user_pass, login_count);
while (current_user_role == "error" && login_count < 3) {
    current_user_role = UserLogin(db_users, current_user_name, current_user_pass, login_count);
}

if (login_count == 3) {
    cout << "\nACCESS NOT GRANTED\nTOO MANY FAILED LOGIN ATTEMPTS\n";
}


 loadProducts(db_products, products);

    if(current_user_role == "manager"){
        User_in_Audit(db_audit, current_user_name, current_user_role);

    do{     
        cout<<" ""<--- PRODUCT MANAGER --->"<<endl;
        cout<<"1. Add new product"<<endl;
        cout<<"2. Update existing products"<<endl;
        cout<<"3. Delete existing product"<<endl;
        cout<<"4. Sell a product"<<endl;
        cout<<"5. Search for a product"<<endl;
        cout<<"6. View all products"<<endl;
        cout<<"7. View sales infographics (work in progress)"<<endl;                                                                               
        cout<<"8. Exit product manager"<<endl;    
        cout<<"Enter action to perform:"<<endl;                                  
        cin>>action;
        

        if(action == 1){
           addProduct(products);
           bubbleSort(products);
           saveProducts(db_products, products);
        }
        else if(action == 2){
            updateProduct(products);
             bubbleSort(products);
             saveProducts(db_products, products);
        }
        else if(action == 3){
            deleteProduct(db_products, products);
            saveProducts(db_products, products);
        }
        else if(action == 4){
            sellProduct(db_sales, products, current_user_name, total_sales);
            saveProducts(db_products, products);
        }
        else if(action == 5){
            do{
            cout<<"<--- SEARCH FOR A PRODUCT --->"<<endl;
            cout<<"1. Search by code"<<endl;
            cout<<"2. Search by brand"<<endl;
            cout<<"3. Search by name"<<endl;
            cout<<"4. Exit"<<endl;
            cout<<"Enter action to perform:"<<endl;
            cin>>sub_action;

            if(sub_action == 1){
                sequentialSearchByCode(products);
            }
            else if(sub_action == 2){
                 sequentialSearchByBrand(products);
            }
            else if(sub_action == 3){
                sequentialSearchByName(products);
            }
        }while(sub_action != 4);
        }
        else if(action == 6){
            AllProducts(products);
        }
        else if(action == 7){
            double total_value;
            SumTotalValue(products, total_value);

        }

    }while(action != 8);

    user_out_audit(db_audit,current_user_name, current_user_role);
    cout<<"Thank you for using product manager"<<endl;
    cout<<" <--- Prodexa Enterprises Ltd --->"<<endl;
}

    if(current_user_role == "cashier"){
        User_in_Audit(db_audit, current_user_name, current_user_role);
    
    do{
        cout<<"1. Sell a product"<<endl;
        cout<<"2. Search for a product"<<endl;
        cout<<"3. View all products"<<endl;
        cout<<"4. Exit product manager"<<endl;
        cout<<"Enter action to perform:"<<endl;
        cin>>action;

        if(action == 1){
            sellProduct(db_sales, products, current_user_name, total_sales);
            saveProducts(db_products, products);
        }

        else if(action == 2){
            do{
            cout<<"<--- SEARCH FOR A PRODUCT --->"<<endl;
            cout<<"1. Search by code"<<endl;
            cout<<"2. Search by brand"<<endl;
            cout<<"3. Search by name"<<endl;
            cout<<"4. Exit"<<endl;
            cout<<"Enter action to perform:"<<endl;
            cin>>sub_action;

            if(sub_action == 1){
                sequentialSearchByCode(products);
            }
            else if(sub_action == 2){
                 sequentialSearchByBrand(products);
            }
            else if(sub_action == 3){
                sequentialSearchByName(products);
            }
        }while(sub_action != 4);
        }
             else if(action == 3){
            AllProducts(products);
        }
    


    }while(action != 4);

    user_out_audit(db_audit, current_user_name, current_user_role);
    cout<<"Thank you for using product manager"<<endl;
    cout<<" <--- Prodexa Enterprises Ltd --->"<<endl;
}

    return 0;
}


/* cd source_code
clang++ product_manager.cpp -o product_manager_app -lsqlite3 -lpthread -ldl && ./product_manager_app */
