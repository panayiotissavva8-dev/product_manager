const logoutBtn = document.getElementById("logoutBtn");
const addProductForm = document.getElementById("addProductForm");

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

// --- Add product form submit ---
addProductForm.addEventListener("submit", async (e) => {
    e.preventDefault();

    const token = localStorage.getItem("sessionToken");
    if (!token) {
        window.location.href = "/";
        return;
    }

    const productData = {
        code: parseInt(document.getElementById("code").value),
        brand: document.getElementById("brand").value,
        name: document.getElementById("name").value,
        quantity: parseInt(document.getElementById("quantity").value),
        stock_alert: parseInt(document.getElementById("stock_alert").value),
        cost: parseFloat(document.getElementById("cost").value),
        price: parseFloat(document.getElementById("price").value),
        discount: parseFloat(document.getElementById("discount").value)
    };

    try {
        const res = await fetch("/add_product", {
            method: "POST",
            headers: { 
                "Content-Type": "application/json",
                "Authorization": token   // <-- Add this
            },
            body: JSON.stringify(productData)
        });

        if (res.ok) {
            alert("Product added successfully!");
            addProductForm.reset();
        } else if (res.status === 409) {
            alert("Product with this code already exists!");
        } else if (res.status === 401) {
            alert("Session expired. Please log in again.");
            localStorage.removeItem("sessionToken");
            window.location.href = "/";
        } else {
            alert("Failed to add product");
        }
    } catch (err) {
        console.error("Add product error:", err);
        alert("Error adding product");
    }
});
