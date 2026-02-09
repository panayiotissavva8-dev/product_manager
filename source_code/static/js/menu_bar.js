fetch("/html/menu_bar.html")
    .then(res => res.text())
    .then(html => {
        document.body.insertAdjacentHTML("afterbegin", html);
    });

document.addEventListener("click", e => {
    const routes = {
        viewProductsBtn: "/products",
        viewaddProductBtn: "/add_product",
        vieweditProductBtn: "/edit_product",
        viewSalesBtn: "/sales",
        viewaddSaleBtn: "/add_sale",
        viewSalesReportBtn: "/sales_report",
        viewUsersBtn: "/view_users",
        viewCustomersBtn: "/view_customers",
        viewaddCustomerBtn: "/add_customers",
        vieweditCustomerBtn: "edit_customers"
    };

    if (routes[e.target.id]) {
        location.href = routes[e.target.id];
    }
});
