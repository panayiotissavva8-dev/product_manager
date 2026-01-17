// ===== Elements =====
const productsTableBody = document.querySelector("#productsTable tbody");
const logoutBtn = document.getElementById("logoutBtn");

// ===== Load Products =====
async function loadProducts() {
    const token = localStorage.getItem("sessionToken");
    if (!token) {
        window.location.href = "/";
        return;
    }

    try {
        const res = await fetch("/api/products", {
            headers: { "Authorization": token }
        });

        if (!res.ok) {
            if (res.status === 401) {
                alert("Session expired. Please log in again.");
                localStorage.removeItem("sessionToken");
                window.location.href = "/";
            } else {
                alert("Failed to load products. Status: " + res.status);
            }
            return;
        }

        const data = await res.json();
        // Works whether backend returns array directly or { products: [...] }
        const products = Array.isArray(data) ? data : data.products || [];

        if (!productsTableBody) {
            console.error("Table body not found!");
            return;
        }

        // Clear previous rows
        productsTableBody.innerHTML = "";

        if (products.length === 0) {
            const row = document.createElement("tr");
            row.innerHTML = `<td colspan="8" style="text-align:center;">No products found</td>`;
            productsTableBody.appendChild(row);
            return;
        }

        // Populate table
        products.forEach(p => {
            const row = document.createElement("tr");

            if (p.quantity < 5) row.classList.add("low-stock");

            row.innerHTML = `
                <td>${p.code}</td>
                <td>${p.brand}</td>
                <td>${p.name}</td>
                <td>${p.quantity}</td>
                <td>${p.stock_alert}</td>
                <td>€${p.cost?.toFixed(2) ?? "0.00"}</td>
                <td>€${p.price?.toFixed(2) ?? "0.00"}</td>
                <td>${p.discount ?? 0}%</td>
            `;
            productsTableBody.appendChild(row);
        });

    } catch (err) {
        console.error("Failed to load products", err);
        alert("Error connecting to backend");
    }
}

// ===== Logout =====
logoutBtn?.addEventListener("click", async () => {
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
            alert("Logout failed");
        }
    } catch (err) {
        console.error("Logout error:", err);
    }
});

// ===== Run =====
document.addEventListener("DOMContentLoaded", loadProducts);
