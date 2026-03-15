document.addEventListener("DOMContentLoaded", async () => {
    const usernameSpan = document.getElementById("username");
    const roleSpan = document.getElementById("role");
    const logoutBtn = document.getElementById("logoutBtn");
    
    // Products Buttons
    const viewProductsBtn = document.getElementById("viewProductsBtn");
    const viewaddProductBtn = document.getElementById("viewaddProductBtn");
    const vieweditProductBtn = document.getElementById("vieweditProductBtn");
    const viewdeleteProductBtn = document.getElementById("viewdeleteProductBtn");

    // Sales Buttons
    const viewSalesBtn = document.getElementById("viewSalesBtn");
    const viewaddSaleBtn = document.getElementById("viewaddSaleBtn");
    const viewdeleteSaleBtn = document.getElementById("viewdeleteSaleBtn");
    const viewSalesReportBtn = document.getElementById("viewSalesReportBtn");

    // Customers Buttons
    const viewCustomersBtn = document.getElementById("viewCustomersBtn");
    const viewaddCustomersBtn = document.getElementById("viewaddCustomersBtn");
    const vieweditCustomersBtn = document.getElementById("vieweditCustomersBtn");
    const viewdeleteCustomersBtn = document.getElementById("viewdeleteCustomersBtn");


    const token = localStorage.getItem("sessionToken");
    if (!token) {
        window.location.href = "/"; // redirect to login if no token
        return;
    }

    //  Fetch current user with token
    try {
        const response = await fetch("/current_user", {
            method: "GET",
            headers: {
                "Content-Type": "application/json",
                "Authorization": token
            }
        });

        if (!response.ok) {
            localStorage.removeItem("sessionToken");
            window.location.href = "/"; // session invalid
            return;
        }

        const data = await response.json();

        usernameSpan.textContent = data.username;
        roleSpan.textContent = data.role;

    } catch (err) {
        console.error("Error fetching user data:", err);
        alert("Failed to connect to backend.");
        return;
    }

    //  NAVIGATION BUTTONS

    // Products Buttons
    if (viewProductsBtn) {
        viewProductsBtn.addEventListener("click", () => window.location.href = "/products");
    }
    if (viewaddProductBtn) {
        viewaddProductBtn.addEventListener("click", () => window.location.href = "/add_product");
    }
    if (vieweditProductBtn) {
        vieweditProductBtn.addEventListener("click", () => window.location.href = "/edit_product");
    }
    if(viewdeleteProductBtn){
        viewdeleteProductBtn.addEventListener("click", () => window.location.href = '/delete_product');
    }

    // Sales Buttons
    if(viewSalesBtn){
        viewSalesBtn.addEventListener("click", () => window.location.href = "/sales");
    }
    if(viewaddSaleBtn){
        viewaddSaleBtn.addEventListener("click", () => window.location.href = "/add_sale");
    }
    if(viewdeleteSaleBtn){
        viewdeleteSaleBtn.addEventListener("click", () => window.location.href = "/delete_sale");
    }
    if(viewSalesReportBtn){
        viewSalesReportBtn.addEventListener("click", () => window.location.href = "/sales_report");
    }

    // Customers Buttons
     if(viewCustomersBtn){
        viewCustomersBtn.addEventListener("click", () =>  window.location.href = "/view_customers");
    }
    if(viewaddCustomersBtn){
        viewaddCustomersBtn.addEventListener("click", () =>  window.location.href = "/add_customers");
    }
    if(vieweditCustomersBtn){
        vieweditCustomersBtn.addEventListener("click", () =>  window.location.href = "/edit_customers");
    }
    if(viewdeleteCustomersBtn){
        viewdeleteCustomersBtn.addEventListener("click", () =>  window.location.href = "/delete_customers");
    }


    //  LOGOUT
    if (logoutBtn) {
        logoutBtn.addEventListener("click", async () => {
            try {
                const response = await fetch("http://localhost:18080/logout", {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json",
                        "Authorization": token
                    }
                });

                if (response.ok) {
                    localStorage.removeItem("sessionToken");
                    window.location.href = "/";
                } else {
                    alert("Logout failed");
                }
            } catch (err) {
                console.error("Error during logout:", err);
            }
        });
    }
});
