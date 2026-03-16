// Load the menu dynamically
fetch("/assets/html/menu_bar.html")
    .then(res => res.text())
    .then(html => {
        document.body.insertAdjacentHTML("afterbegin", html);

        const role = localStorage.getItem("role"); // stored at login

        // --- Hide Users section for Employees ---
        if (role === "Employee") {
            const usersDropdown = document.querySelector('.menu .dropdown:nth-child(4)'); // Users is 4th dropdown
            if (usersDropdown) usersDropdown.style.display = "none";
        }

        // --- Delegated click listener ---
        document.body.addEventListener("click", e => {
            const routes = {
                // Dashboard
                viewDashboardBtn: role === "Employee" ? "/employee_dashboard" : "/dashboard",

                // Product routes
                viewProductsBtn: "/products",
                viewaddProductBtn: "/add_product",
                vieweditProductBtn: "/edit_product",
                viewdeleteProductBtn: "/delete_product",

                // Sales routes
                viewSalesBtn: "/sales",
                viewaddSaleBtn: "/add_sale",
                viewdeleteSaleBtn: "/delete_sale",
                viewSalesReportBtn: "/sales_report",

                // User routes (even though hidden for Employee)
                viewUsersBtn: "/view_users",
                viewaddUserBtn: "/add_users",
                viewdeleteUserBtn: "/delete_users",
                viewviewUseractivity: "/view_audit_users",

                // Customer routes
                viewCustomersBtn: "/view_customers",
                viewaddCustomerBtn: "/add_customers",
                vieweditCustomerBtn: "/edit_customers",
                viewdeleteCustomersBtn: "/delete_customers"
            };

            if (routes[e.target.id]) {
                window.location.href = routes[e.target.id];
            }
        });
    })
    .catch(err => console.error("Failed to load menu:", err));