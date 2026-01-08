#include<iostream>
#include<string>
#include<fstream>
#include<iomanip>
#include<cmath>
#include<ctime>
#include <sqlite3.h>
using namespace std;
sqlite3* db;


// =============================================================
// ===================== SQLITE SECTION =========================
// =============================================================

/*sqlite3* db;

// Open database connection
void initDB() {
if (sqlite3_open("products.db", &db) != SQLITE_OK) {
cout << "Failed to open database" << endl;
}
}

// Close database
void closeDB() {
sqlite3_close(db);
}

// Create table if it does not exist
void createTables() {
const char* sql =
"CREATE TABLE IF NOT EXISTS products ("
"code INTEGER PRIMARY KEY,"
"brand TEXT,"
"name TEXT,"
"quantity INTEGER,"
"stock_alert INTEGER,"
"cost REAL,"
"price REAL,"
"discount REAL"
");";

sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
} */

 //loads all products from file
  void loadProducts(int code[],string brand[], string name[], int quantity[], double cost[], double price[], double discount[], int &count, int stock_alert[]){
    ifstream fin("/Users/panayiotissavva/Documents/product_manager/files/products.txt");
    count = 0;

    while(fin >> code[count]){
        fin.ignore(); // skip space before the opening quote
        char quote;
        fin >> quote;
        getline(fin, brand[count], '"');                                  
        fin.ignore();
        fin>>quote;
        getline(fin, name[count], '"'); // read until closing quote
        fin >> quantity[count] >> stock_alert[count] >> cost[count] >> price[count] >> discount[count];
        count++;
    }
    fin.close();
} 

//saves all products to file
 void saveProducts(int code[], string brand[], string name[], int quantity[], double cost[], double price[], double discount[], int count, int stock_alert[]){
    ofstream fout("/Users/panayiotissavva/Documents/product_manager/files/products.txt", ios::trunc); // overwrite file
    for(int i=0; i<count; i++){
        fout << code[i] <<" \""<<brand[i]<<"\" "<< " \""<< name[i] << "\" " << quantity[i] << " " << stock_alert[i] << " " << cost[i] << " " << price[i] <<" " << discount[i] << endl;
    }
    fout.close();            
} 

// =============================================================
// =============================================================

// This function REPLACES loadProducts(), but keeps same signature
/* void loadProducts(int code[], string brand[], string name[], int quantity[],
double cost[], double price[], double discount[],
int& count, int stock_alert[]) {

const char* sql = "SELECT * FROM products";
sqlite3_stmt* stmt;

count = 0;
sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

while (sqlite3_step(stmt) == SQLITE_ROW) {
code[count] = sqlite3_column_int(stmt, 0);
brand[count] = (const char*)sqlite3_column_text(stmt, 1);
name[count] = (const char*)sqlite3_column_text(stmt, 2);
quantity[count] = sqlite3_column_int(stmt, 3);
stock_alert[count] = sqlite3_column_int(stmt, 4);
cost[count] = sqlite3_column_double(stmt, 5);
price[count] = sqlite3_column_double(stmt, 6);
discount[count] = sqlite3_column_double(stmt, 7);
count++;
}

sqlite3_finalize(stmt);
}

// This function REPLACES saveProducts(), database auto-saves
void saveProducts(...) {
// SQLite automatically persists data
// This function intentionally left blank
}

// =============================================================
// ================== UPDATED BUSINESS LOGIC ====================
// =============================================================

void addProduct(int code, string brand, string name, int quantity,
int stock_alert, double cost, double price, double discount) {

const char* sql =
"INSERT INTO products VALUES (?,?,?,?,?,?,?,?)";

sqlite3_stmt* stmt;
sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

sqlite3_bind_int(stmt, 1, code);
sqlite3_bind_text(stmt, 2, brand.c_str(), -1, SQLITE_TRANSIENT);
sqlite3_bind_text(stmt, 3, name.c_str(), -1, SQLITE_TRANSIENT);
sqlite3_bind_int(stmt, 4, quantity);
sqlite3_bind_int(stmt, 5, stock_alert);
sqlite3_bind_double(stmt, 6, cost);
sqlite3_bind_double(stmt, 7, price);
sqlite3_bind_double(stmt, 8, discount);

sqlite3_step(stmt);
sqlite3_finalize(stmt);
}

void sellProduct(int code, int qty) {
const char* sql =
"UPDATE products SET quantity = quantity - ? "
"WHERE code = ? AND quantity >= ?";

sqlite3_stmt* stmt;
sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

sqlite3_bind_int(stmt, 1, qty);
sqlite3_bind_int(stmt, 2, code);
sqlite3_bind_int(stmt, 3, qty);

sqlite3_step(stmt);

if (sqlite3_changes(db) == 0)
cout << "Not enough stock or product not found" << endl;
else
cout << "Product sold successfully" << endl;

sqlite3_finalize(stmt);
}
*/

//lets user add new product
void addProduct(int code[], string brand[], string name[], int quantity[], double cost[], double price[], double discount[], int &count, int stock_alert[]){
    int new_code;
    string new_brand;
    string new_name;
    int new_quantity;
    double new_cost; 
    double new_price;
    double new_discount;
    int new_stock_alert;
    bool exists=false;
    cout<<" ""<--- NEW PRODUCT --->"<<endl;
    cout<<"Enter product code:"<<endl;
    cin>>new_code;
    for(int i=0; i<count; i++){
        if(new_code == code[i]){
            cout<<"A product with this code already exists"<<endl;
            cout<<"          "<<"Try again"<<endl;
            exists = true;
            break;
        }
    }
    if(exists == true){
        return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout<<"Enter brand name:"<<endl; 
    getline(cin, new_brand);

    cout<<"Enter product name:"<<endl;
    getline(cin,new_name);

    for(int i=0; i<count; i++){
        if(new_name == name[i]){
            cout<<"A product with this name already exists"<<endl;
            cout<<"          "<<"Try again"<<endl;
            exists = true;
            break;
        }
    }
    if(exists == true){
        return;
    }

    cout<<"Enter product quantity:"<<endl;
    cin>>new_quantity;

    cout<<"Enter product quantity alert:"<<endl;
    cin>>new_stock_alert;

    cout<<"Enter product cost:"<<endl;
    cin>>new_cost;

    cout<<"Enter product price:"<<endl;
    cin>>new_price;

    cout<<"Enter product discount"<<endl;
    cin>>new_discount;

    code[count] = new_code;
    brand[count] = new_brand;
    name[count] = new_name;
    quantity[count] = new_quantity;
    stock_alert[count] = new_stock_alert;
    cost[count] = new_cost;
    price[count] = new_price;
    discount[count] = new_discount;

 ofstream fout("/Users/panayiotissavva/Documents/product_manager/files/products.txt", ios::app);
 fout << new_code <<" \""<< new_brand <<"\" "<< " \"" << new_name << "\" " << new_quantity << " " << new_stock_alert << " " <<new_cost << " " << new_price << " " << new_discount << endl;
 fout.close(); 


count++;

}

//lets user delete product
void deleteProduct(int code[], string brand[], string name[], int quantity[], double cost[], double price[], double discount[], int &count, int stock_alert[]){
    int target;
    cout<<" ""<--- DELETE PRODUCT --->"<<endl;
    cout<<"Enter product code:"<<endl;
    cin>>target;

    bool found = false;
    for(int i=0; i<count; i++){
        if(target == code[i]){
            found = true;
    for(int j=i; j<count-1; j++){
            code[j] = code[j+1];
            brand[j] = brand[j+1];
            name[j] = name[j+1];
            quantity[j] = quantity[j+1];
            stock_alert[j] = stock_alert[j+1];
            cost[j] = cost[j+1];
            price[j] = price[j+1];
            discount[j] = discount[j+1];
    }
            count--;
            cout<<"Product deleted"<<endl;
            break;
          }
        }
        if(found == false){
            cout<<"Product with code "<<target<<" not found"<<endl;
        }
    }

//lets user update an already existing product
void updateProduct(int code[], string brand[], string name[], int quantity[], double cost[], double price[], double discount[], int count, int stock_alert[]){
    int target_code;
    string update_brand;
    string update_name;
    int update_quantity;
    int update_stock_alert;
    double update_cost;
    double update_price;
    double update_discount;
    int action=0;
    cout<<" ""<--- UPDATE PRODUCT --->"<<endl;
    cout<<"Enter product code"<<endl;
    cin>>target_code;
    for(int i=0; i<count; i++){
        if((target_code == code[i]) && (code[i] != -1)){
            
            
            do{
            cout<<"Select action to perform:"<<endl;
            cout<<"1. Update brand"<<endl;
            cout<<"2. Update name"<<endl;
            cout<<"3. Update quantity"<<endl;
            cout<<"4. Update stock alert"<<endl;
            cout<<"5. Update cost"<<endl;
            cout<<"6. Update price"<<endl;
            cout<<"7. Update discount"<<endl;
            cout<<"8. Exit"<<endl;
            cin>>action;
            
            if(action == 1){
                cout<<"Enter new brand name:"<<endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                getline(cin, update_brand);
                brand[i]=update_brand;
            }
           else if(action == 2){
                cout<<"Enter new product name:"<<endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                getline(cin, update_name);
                name[i]=update_name;
            }
            else if(action == 3){
                cout<<"Enter new product quantity:"<<endl;
                cin>>update_quantity;
                quantity[i]=update_quantity;
            }
            else if(action == 4){
                cout<<"Enter new product stock alert:"<<endl;
                cin>>update_stock_alert;
                stock_alert[i]=update_stock_alert;
            }
            else if(action == 5){
                cout<<"Enter new product cost:"<<endl;
                cin>>update_cost;
                cost[i]=update_cost;
            }
            else if(action == 6){
                cout<<"Enter new product price:"<<endl;
                cin>>update_price;
                price[i]=update_price;
            }
            else if(action == 7){
                cout<<"Enter new product discount:"<<endl;
                cin>>update_discount;
                discount[i]=update_discount;
            }
            } while(action != 8);
        }
     }
       cout<<"Total products:"<<count<<endl;
}

//lets user sell a product and records it to a sales audit 
void sellProduct(int code[], string brand[], string name[], int quantity[], double cost[], double price[], double discount[], int count, double &total_value, int stock_alert[], string current_user_name, int &total_sales){
    ofstream sale_audit("/Users/panayiotissavva/Documents/product_manager/files/sales.txt",ios::app);
    ofstream sum_sales("/Users/panayiotissavva/Documents/product_manager/files/sum_sales.txt",ios::app);


    time_t now = time(0);
    char* dt = ctime(&now);

    int target;
    int target_count;
    int amount_to_sell;
    int answer;
    double value_sold;
    cout<<"<--- SELL PRODUCT --->"<<endl;
    cout<<"Enter product code to sell:"<<endl;
    cin>>target;

    for(int i=0; i<count; i++){
        if(target == code[i]){
            target_count = i;
        }
    }
    cout<<"Is this the correct product:"<<endl;
    cout<<endl;
    cout<<"CODE:" << code[target_count] << " " << "NAME:" << name[target_count] << " " << "PRICE:" << price[target_count] <<endl;
    cout<<endl;
    cout<<"1. YES"<<endl;
    cout<<"2. NO"<<endl;
    cin>>answer;

    if(answer == 1){
        cout<<"How many untis do you want to sell:"<<endl;
        cin>>amount_to_sell;

        if(quantity[target_count] == 0){
            cout<<"Product is out of stock"<<endl;
        }

       else if((quantity[target_count] - amount_to_sell) < 0){
            cout<<"Cannot sell " << amount_to_sell <<" units of this product since only " << quantity[target_count] <<" units are in stock" <<endl;
        }

        else{
        quantity[target_count] = quantity[target_count] - amount_to_sell;
        }
        value_sold = amount_to_sell * price[target_count];
        sale_audit << dt << " " << "USER:" << current_user_name << " " << "CODE:" << code[target_count] << " " << "PRODUCT:" << name[target_count] << " " << "QUANTITY SOLD:" << amount_to_sell << " " << "PRICE:" << price[target_count] << " " << "TOTAL VALUE:" << fixed << setprecision(2) << value_sold <<endl;
        total_sales++;
        sum_sales << code[target_count] << " \"" << name[target_count] << "\" " << amount_to_sell << " " << price[target_count] << " " << fixed << setprecision(2) << value_sold <<endl;
        cout<<endl;
        if(quantity[target_count] <= stock_alert[target_count]){
        cout<<"WARNING:product quantity is below "<<stock_alert[target_count]<<endl;
        cout<<endl;
        }
    }
    
    else{
        cout<<"Try again"<<endl;
    }
    // cout<<"quantity: "<<quantity[target_count];

    sale_audit.close();
    sum_sales.close();
}


// work in progress
//do not use SumSold function!!!
void SumSold(int code[], string brand[], string name[], int quantity[], double cost[], double price[], double discount[], int count, double &total_value, int stock_alert[], string current_user_name, int total_sales, int &sum_sold){
    ifstream sales_in("/Users/panayiotissavva/Documents/product_manager/files/sum_sales.txt");

    string product_brand[1000];
    int sold_quantity[1000];
    int product_code[1000];
    string product_name[1000];
    double product_price[1000];
    double sum_value[1000];
    
    string top_product_brand;
    int top_product_code;                           
    string top_product_name;
    double top_product_price;
    double top_sum_value;
    int top_product_quantity;

    char* dt;

    if (!sales_in.is_open()) {
    cout << "Failed to open sum_sales.txt" << endl;
    return;
}

    int i = 0;
    int sales_rec;

    while(sales_in >> product_code[i]){
        sales_in >> product_brand[i];
        sales_in.ignore();
        char quote;
        sales_in >> quote;
        getline(sales_in, product_name[i], '"');
        sales_in.ignore();
        sales_in >> sold_quantity[i] >> product_price[i] >> sum_value[i];
        i++;
       sales_rec = i;    
    }

    // cout<<sales_rec<<endl;
    int max_sold = 0;
    sum_sold = 0;

    for(int i=0; i<sales_rec; i++){
        sum_sold += sold_quantity[i];

        if(sold_quantity[i] > max_sold){
            top_product_brand = product_brand[i];
            max_sold = sold_quantity[i];
            top_product_code = product_code[i];
            top_product_name = product_name[i];
            top_product_price = product_price[i];
            top_product_quantity = sold_quantity[i];
            top_sum_value = sum_value[i];
        }
    }
    cout<<"<--- SALES INFOGRAPHICS --->"<<endl;
    cout<<"Total Products sold:" << sales_rec <<endl;
    cout<<"Best selling product with code" << " " << top_product_code << " " << "is" << " " << top_product_brand << " " << top_product_name << " " << "with" << " " << top_product_quantity << " " << "units sold with a price per unit of $" << " " << top_product_price << " " << "and total sell value of $" << " " << top_sum_value <<endl;

    sales_in.close();
}

// lets user search for a product by its code
void sequentialSearchByCode(int code[], string brand[], string name[], int quantity[], double cost[], double price[], double discount[], int count, int stock_alert[]){
    int target;
    double total_cost[count];
    double value[count];
    bool found = false;
    cout<<"Enter product code:"<<endl;
    cin>>target;
    for(int i=0; i<count; i++){
        if(target == code[i]){
           total_cost[i] = quantity[i] * cost[i];
           if(discount[i] != 0){
            value[i] = ((quantity[i] * price[i]) - (discount[i] * quantity[i]));
           }
           else{
            value[i] = (quantity[i] * price[i]);
           }

           cout<<left<<setw(8)<<"CODE";
           cout<<setw(15)<<"BRAND";
           cout<<setw(18)<<"NAME";
           cout<<setw(10)<<"QUANTITY";
           cout<<setw(13)<<"STOCK ALERT";
           cout<<setw(12)<<"COST";
           cout<<setw(12)<<"PRICE";
           cout<<setw(12)<<"DISCOUNT";
           cout<<setw(18)<<"TOTAL COST VALUE";
           cout<<setw(18)<<"TOTAL SELL VALUE"<<endl;

           cout<<left<<setw(8)<<code[i];
           cout<<setw(15)<<brand[i];
           cout<<setw(18)<<name[i];
           cout<<setw(10)<<quantity[i];
           cout<<setw(13)<<stock_alert[i];
           cout<<setw(12)<<fixed<<setprecision(2)<<cost[i];
           cout<<setw(12)<<fixed<<setprecision(2)<<price[i];
           cout<<setw(12)<<fixed<<setprecision(2)<<discount[i];
           cout<<setw(18)<<fixed<<setprecision(2)<<total_cost[i];
           cout<<setw(18)<<fixed<<setprecision(2)<<value[i]<<endl;

           if(quantity[i] <= stock_alert[i]){
            cout<<endl;
        cout<<"WARNING:" << name[i] << " " << "product quantity is below "<<stock_alert[i]<<endl;
        cout<<endl;
        }

            found = true;
        }
    }
    if (found == false){
         cout<<"None products found with "<<target<<" as code"<<endl;
    }
}

// lets user search for a product by its brand
void sequentialSearchByBrand(int code[], string brand[], string name[], int quantity[], double cost[], double price[], double discount[], int count, int stock_alert[]){
    string target;
    double total_cost[count];
    double value[count];
    bool found = false;
    cout<<"Enter product brand:"<<endl;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, target);
    for(int i=0; i<count; i++){
        if(target == brand[i]){
           total_cost[i] = quantity[i] * cost[i];
           if(discount[i] != 0){
            value[i] = ((quantity[i] * price[i]) - (discount[i] * quantity[i]));
           }
           else{
            value[i] = (quantity[i] * price[i]);
           }

           cout<<left<<setw(8)<<"CODE";
           cout<<setw(15)<<"BRAND";
           cout<<setw(18)<<"NAME";
           cout<<setw(10)<<"QUANTITY";
           cout<<setw(13)<<"STOCK ALERT";
           cout<<setw(12)<<"COST";
           cout<<setw(12)<<"PRICE";
           cout<<setw(12)<<"DISCOUNT";
           cout<<setw(18)<<"TOTAL COST VALUE";
           cout<<setw(18)<<"TOTAL SELL VALUE"<<endl;

           cout<<left<<setw(8)<<code[i];
           cout<<setw(15)<<brand[i];
           cout<<setw(18)<<name[i];
           cout<<setw(10)<<quantity[i];
           cout<<setw(13)<<stock_alert[i];
           cout<<setw(13)<<fixed<<setprecision(2)<<cost[i];
           cout<<setw(12)<<fixed<<setprecision(2)<<price[i];
           cout<<setw(12)<<fixed<<setprecision(2)<<discount[i];
           cout<<setw(18)<<fixed<<setprecision(2)<<total_cost[i];
           cout<<setw(18)<<fixed<<setprecision(2)<<value[i]<<endl;

           if(quantity[i] <= stock_alert[i]){
            cout<<endl;
        cout<<"WARNING:" << name[i] << " " << "product quantity is below "<<stock_alert[i]<<endl;
        cout<<endl;
        }

            found = true;
        }
    }
    if (found == false){
         cout<<"None products found with "<<target<<" as brand"<<endl;
    }


}

// lets user search for a product by its name
void sequentialSearchByName(int code[], string brand[], string name[], int quantity[], double cost[], double price[], double discount[], int count, int stock_alert[]){
     string target;
     double total_cost[count];
     double value[count];
     bool found = false;
     cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout<<"Enter product name:"<<endl;
    getline(cin, target);
    for(int i=0; i<count; i++){
        if(target == name[i]){
           total_cost[i] = quantity[i] * cost[i];
           if(discount[i] != 0){
            value[i] = ((quantity[i] * price[i]) - (discount[i] * quantity[i]));
           }
           else{
            value[i] = (quantity[i] * price[i]);
           }

           cout<<left<<setw(8)<<"CODE";
           cout<<setw(15)<<"BRAND";
           cout<<setw(18)<<"NAME";
           cout<<setw(10)<<"QUANTITY";
           cout<<setw(13)<<"STOCK ALERT";
           cout<<setw(12)<<"COST";
           cout<<setw(12)<<"PRICE";
           cout<<setw(12)<<"DISCOUNT";
           cout<<setw(18)<<"TOTAL COST VALUE";
           cout<<setw(18)<<"TOTAL SELL VALUE"<<endl;

           cout<<left<<setw(8)<<code[i];
           cout<<setw(15)<<brand[i];
           cout<<setw(18)<<name[i];
           cout<<setw(10)<<quantity[i];
           cout<<setw(13)<<stock_alert[i];
           cout<<setw(13)<<fixed<<setprecision(2)<<cost[i];
           cout<<setw(12)<<fixed<<setprecision(2)<<price[i];
           cout<<setw(12)<<fixed<<setprecision(2)<<discount[i];
           cout<<setw(18)<<fixed<<setprecision(2)<<total_cost[i];
           cout<<setw(18)<<fixed<<setprecision(2)<<value[i]<<endl;

           if(quantity[i] <= stock_alert[i]){
            cout<<endl;
        cout<<"WARNING:" << name[i] << " " << "product quantity is below "<<stock_alert[i]<<endl;
        cout<<endl;
        }

            found = true;
        }
    }
    if (found == false){
        cout<<"None products found with "<<target<<" as name"<<endl;
    }
}

// sorts all products using its code from smallest to biggest
void bubbleSort(int code[], string brand[], string name[], int quantity[], double cost[], double price[], double discount[], int count, int stock_alert[]){
    int temp_code, temp_quantity, temp_stock_alert;
    string temp_brand;
    string temp_name;
    double temp_cost;
    double temp_price;
    double temp_discount;
    bool sorted;

    do{
        sorted = true;
        for(int i=0; i<count - 1; i++){
            if(code[i] > code[i+1]){
                temp_code = code[i];
                temp_brand = brand[i];
                temp_quantity = quantity[i];
                temp_stock_alert = stock_alert[i];
                temp_name = name[i];
                temp_cost = cost[i];
                temp_price = price[i];
                temp_discount = discount[i];

                code[i] = code[i+1];
                brand[i] = brand[i+1];
                name[i] = name[i+1];
                quantity[i] = quantity[i+1];
                stock_alert[i] = stock_alert[i+1];
                cost[i] = cost[i+1];
                price[i] = price[i+1];
                discount[i] = discount[i+1];

                code[i+1] = temp_code;
                brand[i+1] = temp_brand;
                name[i+1] = temp_name;
                quantity[i+1] = temp_quantity;
                stock_alert[i+1] = temp_stock_alert;
                cost[i+1] = temp_cost;
                price[i+1] = temp_price;
                discount[i+1] = temp_discount;
                sorted = false;
            }
        }
    } while(sorted == false);

}

//calculates the total sell value of all products 
void SumTotalValue(int code[], string brand[], string name[], int quantity[], double cost[], double price[], double discount[], int count, double &total_value){
    total_value=0.0;
    for(int i=0; i < count; i++){
    total_value = total_value + (quantity[i] * price[i]);
}
cout<<"Total sell value of all products:"<<fixed<<setprecision(2)<<total_value<<endl;
}

//lets user see all products and their information
void AllProducts(int code[], string brand[], string name[], int quantity[], double cost[], double price[], double discount[], int count, double &total_value, int stock_alert[]){
     total_value = 0.00;
     double total_cost[count];
     double value[count];

    cout<<"  ""<--- ALL PRODUCTS --->"<<endl;
    cout<<left<<setw(8)<<"CODE";
    cout<<setw(15)<<"BRAND";
    cout<<setw(18)<<"NAME";
    cout<<setw(10)<<"QUANTITY";
    cout<<setw(13)<<"STOCK ALERT";
    cout<<setw(12)<<"COST";
    cout<<setw(12)<<"PRICE";
    cout<<setw(12)<<"DISCOUNT";
    cout<<setw(18)<<"TOTAL COST VALUE";
    cout<<setw(18)<<"TOTAL SELL VALUE"<<endl;
    

     for(int i=0; i < count; i++){
    total_cost[i] = quantity[i] * cost[i];
    if(discount[i] != 0){
    value[i] += ((quantity[i] * price[i]) - (discount[i] * quantity[i]));
    }
    else {
    value[i] += (quantity[i] * price[i]);
    }

    total_value += (quantity[i] * price[i]); 
    cout<<left<<setw(8)<<code[i];
    cout<<setw(15)<<brand[i];
    cout<<setw(18)<<name[i];
    cout<<setw(10)<<quantity[i];
    cout<<setw(13)<<stock_alert[i];
    cout<<setw(13)<<fixed<<setprecision(2)<<cost[i];
    cout<<setw(12)<<fixed<<setprecision(2)<<price[i];
    cout<<setw(12)<<fixed<<setprecision(2)<<discount[i];
    cout<<setw(18)<<fixed<<setprecision(2)<<total_cost[i];
    cout<<setw(18)<<fixed<<setprecision(2)<<value[i]<<endl;

}

for(int i=0; i<count; i++){
if(quantity[i] <= stock_alert[i]){
            cout<<endl;
        cout<<"WARNING:" << name[i] << " " << "stock quantity is below "<< " " <<stock_alert[i]<<endl;
        cout<<endl;
    }
}

cout<<"Total products:"<<count<<endl;
cout<<"Total value of all products:"<<fixed<<setprecision(2)<<total_value<<endl;

}

//gets user data from file
void LoadUsers(string username[],string password[], string role[], int &user_count){
    ifstream data_in("/Users/panayiotissavva/Documents/product_manager/files/user_data.txt");
    int i=0;
        while(data_in >> username[i] >> password[i] >> role[i]){
    i++;
    user_count++;
    }

    data_in.close();
}

//requires user to login to use features depending on position
string UserLogin(string username[], string password[], string role[], string &current_user_name, string &current_user_pass, int user_count, int &login_count){
    string current_user_role;

     for(int j=0; j<3; j++){
        cout<<"Enter username:"<<endl;
        cin>>current_user_name;
        cout<<"Enter password:";
        cin>>current_user_pass;

    for(int i=0; i<user_count; i++){
        if((current_user_name == username[i]) && (current_user_pass == password[i])){
            current_user_role = role[i];
            current_user_name = username[i];
            current_user_pass = password[i];

            return current_user_role;
        }
    }

         
            login_count++;
            cout<<"Wrong username or password"<<endl;

    }

        cout<<"Too many failed attempts try again later"<<endl;
        current_user_role = "error";
        return current_user_role;

}

//logs in file the time and user information when they entered 
void User_in_Audit(string current_user_name, string current_user_pass, string current_user_role, int user_count){
    ofstream audit_off("/Users/panayiotissavva/Documents/product_manager/files/audit_logs.txt",ios::app);

    time_t now = time(0);
    char* dt = ctime(&now);

    audit_off << "User:" << current_user_name << " " <<"Role:" << current_user_role << " " << "logged in at:" << dt;

}

//logs in file the time and user information when they left
void user_out_audit(string current_user_name, string current_user_pass, string current_user_role, int user_count){
    ofstream audit_off("/Users/panayiotissavva/Documents/product_manager/files/audit_logs.txt",ios::app);

    time_t now = time(0);
    char* dt = ctime(&now);

    audit_off << "User:" << current_user_name << " " <<"Role:" << current_user_role << " " << "logged out at:" << dt;

}

//only use for debug,inclose in comment during normal use
 void userDebug(string username[], string password[], string role[], int user_count){
    for(int i=0; i<user_count; i++){
        cout<<"usernames:"<<username[i]<<" ";
        cout<<"passwords:"<<password[i]<<" ";
        cout<<"users role:"<<role[i]<<" ";
        cout<<"user count:"<<user_count<<endl;
    }
}

 /* void debugPrint(int code[], string brand[], string name[], int quantity[], double cost[], double price[], int count) {
    cout << "DEBUG DUMP -- count=" << count << endl;
    for(int i=0;i<count;i++){
        cout << i << ": code="<<code[i]
             << " name=\"" << name[i] << "\""
             << " qty="<<quantity[i]
             << " cost="<<fixed<<setprecision(2)<<cost[i]
             << " price="<<fixed<<setprecision(2)<<price[i] << endl;
    }
    cout << "---- end debug ----" << endl; 
    }    */

//cals each function depending on user input
int main(){
    int code[1000];
    string brand[1000];
    string name[1000];
    int quantity[1000];
    int stock_alert[1000];
    double cost[1000];
    double price[1000];
    double discount[1000];
    int action=0;
    int sub_action=0;
    int count=0;
    double total_value;
    string username[1000], password[1000], role[1000];
    string current_user_name, current_user_pass, current_user_role;
    int user_count = 0;
    int login_count = 0;
    int total_sales=0;
    int sum_sold = 0;

  loadProducts(code,brand,name,quantity,cost,price,discount,count,stock_alert);
    LoadUsers(username,password,role,user_count);
    userDebug(username,password,role,user_count); 

    current_user_role = UserLogin(username,password,role,current_user_name,current_user_pass,user_count,login_count);

    int i=0;
    while((current_user_role == "error") && (login_count != 3)){
         UserLogin(username,password,role,current_user_name,current_user_pass,user_count,login_count);
         if(login_count == 3){
            cout<<endl;
            cout<<"    ""ACCESS NOT GRANDED"<<endl;
            cout<<"TO MANY FAILED LOGGIN ATTEMPTS"<<endl;
         }
    }
    i++;

   // debugPrint(code,brand,name,quantity,cost,price,count);

    if(current_user_role == "manager"){
        User_in_Audit(current_user_name, current_user_pass, current_user_role, user_count);

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
            addProduct(code,brand,name,quantity,cost,price,discount,count,stock_alert);
         // debugPrint(code,brand,name,quantity,cost,price,count);
            bubbleSort(code,brand,name,quantity,cost,price,discount,count,stock_alert);
        }
        else if(action == 2){
            updateProduct(code,brand,name,quantity,cost,price,discount,count,stock_alert);
             bubbleSort(code,brand,name,quantity,cost,price,discount,count,stock_alert);
             saveProducts(code,brand,name,quantity,cost,price,discount,count,stock_alert);
        }
        else if(action == 3){
            deleteProduct(code,brand,name,quantity,cost,price,discount,count,stock_alert);
             bubbleSort(code,brand,name,quantity,cost,price,discount,count,stock_alert);
             saveProducts(code,brand,name,quantity,cost,price,discount,count,stock_alert);
        }
        else if(action == 4){
            sellProduct(code,brand,name,quantity,cost,price,discount,count,total_value,stock_alert,current_user_name,total_sales);
            saveProducts(code,brand,name,quantity,cost,price,discount,count,stock_alert);
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
                sequentialSearchByCode(code,brand,name,quantity,cost,price,discount,count,stock_alert);
            }
            else if(sub_action == 2){
                 sequentialSearchByBrand(code,brand,name,quantity,cost,price,discount,count,stock_alert);
            }
            else if(sub_action == 3){
                sequentialSearchByName(code,brand,name,quantity,cost,price,discount,count,stock_alert);
            }
        }while(sub_action != 4);
        }
        else if(action == 6){
            AllProducts(code,brand,name,quantity,cost,price,discount,count,total_value,stock_alert);
        }
        else if(action == 7){
            // work in progress
            //do not use SumSold function!!!
            SumSold(code,brand,name,quantity,cost,price,discount,count,total_value,stock_alert,current_user_name,total_sales,sum_sold);
        }

    }while(action != 8);

    user_out_audit(current_user_name, current_user_pass, current_user_role, user_count);
    cout<<"Thank you for using product manager"<<endl;
    cout<<" <--- P.savva Enterprises Ltd --->"<<endl;
}

    if(current_user_role == "cashier"){
        User_in_Audit(current_user_name, current_user_pass, current_user_role, user_count);
    
    do{
        cout<<"1. Sell a product"<<endl;
        cout<<"2. Search for a product"<<endl;
        cout<<"3. View all products"<<endl;
        cout<<"4. Exit product manager"<<endl;
        cout<<"Enter action to perform:"<<endl;
        cin>>action;

        if(action == 1){
            sellProduct(code,brand,name,quantity,cost,price,discount,count,total_value,stock_alert,current_user_name,total_sales);
            saveProducts(code,brand,name,quantity,cost,price,discount,count,stock_alert);
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
                sequentialSearchByCode(code,brand,name,quantity,cost,price,discount,count,stock_alert);
            }
            else if(sub_action == 2){
                 sequentialSearchByBrand(code,brand,name,quantity,cost,price,discount,count,stock_alert);
            }
            else if(sub_action == 3){
                sequentialSearchByName(code,brand,name,quantity,cost,price,discount,count,stock_alert);
            }
        }while(sub_action != 4);
        }
             else if(action == 3){
            AllProducts(code,brand,name,quantity,cost,price,discount,count,total_value,stock_alert);
        }
    


    }while(action != 4);

    user_out_audit(current_user_name, current_user_pass, current_user_role, user_count);
    cout<<"Thank you for using product manager"<<endl;
    cout<<" <--- P.savva Enterprises Ltd --->"<<endl;
}

    return 0;
}