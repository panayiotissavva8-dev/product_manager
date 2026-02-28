fetch("/html/menu_bar.html")
    .then(res => res.text())
    .then(html => {
        document.body.insertAdjacentHTML("afterbegin", html);
    });

document.addEventListener("click", e => {
    const routes = {
        // Dashboard route
        viewDashboardBtn: "/dashboard",

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

        // User routes
        viewUsersBtn: "/view_users",

        // Customer routes
        viewCustomersBtn: "/view_customers",
        viewaddCustomerBtn: "/add_customers",
        vieweditCustomerBtn: "edit_customers"
    };

    if (routes[e.target.id]) {
        location.href = routes[e.target.id];
    }
});
