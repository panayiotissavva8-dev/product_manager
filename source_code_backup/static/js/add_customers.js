document.addEventListener("DOMContentLoaded", () => {
const logoutBtn = document.getElementById("logoutBtn");
const addCustomerForm = document.getElementById("addCustomerForm");

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

// --- Add customer form submit ---
addCustomerForm.addEventListener("submit", async (e) => {
    e.preventDefault();

    const token = localStorage.getItem("sessionToken");
    if (!token) {
        window.location.href = "/";
        return;
    }

    const CustomerData = {
        customer: document.getElementById("customer").value,
        type: document.getElementById("type").value,
        email:document.getElementById("email").value,
        phone: document.getElementById("phone").value,
        address:document.getElementById("address").value,
        postCode:document.getElementById("postCode").value
    };

    try {
        const res = await fetch("/add_customer", {
            method: "POST",
            headers: { 
                "Content-Type": "application/json",
                "Authorization": token   
            },
            body: JSON.stringify(CustomerData)
        });

        if (res.ok) {
            alert("Customer added successfully!");
            addCustomerForm.reset();
        } else if (res.status === 409) {
            alert("Customer already exists!");
        } else if (res.status === 401) {
            alert("Session expired. Please log in again.");
            localStorage.removeItem("sessionToken");
            window.location.href = "/";
        } else {
            alert("Failed to add customer");
        }
    } catch (err) {
        console.error("Add customer error:", err);
        alert("Error adding customer");
    }
});
});