const logoutBtn = document.getElementById("logoutBtn");
const addSaleForm = document.getElementById("addSaleForm");


// --- Logout ---
logoutBtn.addEventListener("click", async () => {
    const token = localStorage.getItem("sessionToken");

    if (!token) {
        window.location.href = "/";
        return;
    }

    try {
        const res = await fetch("/logout", {
            method: "POST",
            headers: { "Authorization": token }
        });

        if (res.ok) {
            localStorage.removeItem("sessionToken");
            window.location.href = "/";
        } else {
            console.error("Logout failed on server");
        }
    } catch (err) {
        console.error("Logout error:", err);
    }
});
// --- Add sale form submit ---
addSaleForm.addEventListener("submit", async (e) => {
    e.preventDefault();

    const token = localStorage.getItem("sessionToken");
    if (!token) {
        window.location.href = "/";
        return;
    }

    const saleData = {
        code: parseInt(document.getElementById("code").value),
        brand: document.getElementById("brand").value,
        name: document.getElementById("name").value,
        quantity: parseInt(document.getElementById("quantity").value),
        price: parseFloat(document.getElementById("price").value),
        discount: parseFloat(document.getElementById("discount").value),
        cost: parseFloat(document.getElementById("cost").value)
        
    };

    try {
        const res = await fetch("/add_sale", {
            method: "POST",
            headers: { 
                "Content-Type": "application/json",
                "Authorization": token  
            },
            body: JSON.stringify(saleData)
        });

        if (res.status === 200) {
            alert("Sale added successfully!");
            addSaleForm.reset();
        } else if (res.status === 401) {
            alert("Session expired. Please log in again.");
            localStorage.removeItem("sessionToken");
            window.location.href = "/";
        } else if (res.status === 500) {
            alert("Failed to add Sale: Insufficient stock");
        }else if(res.status === 201) {
            alert("Sale added successfully with low stock warning.");
            addSaleForm.reset();
        } else if(res.status === 404) {
            alert("Product not found or invalid sale data.");
        }else {
            alert("Failed to add Sale");
        }
    } catch (err) {
        console.error("Add sale error:", err);
        alert("Error adding sale");
    }
});
