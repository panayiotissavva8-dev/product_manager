async function loadSales() {
    console.log("view_sales.js loaded");
    const token = localStorage.getItem("sessionToken");
    if (!token) {
        window.location.href = "/";
        return;
    }

    const salesTable = document.getElementById("salesTable");
    if (!salesTable) {
        console.error("Sales table not found!");
        return;
    }

    const salesTableBody = salesTable.querySelector("tbody");
    if (!salesTableBody) {
        console.error("Table tbody not found!");
        return;
    }

    try {
        const res = await fetch("/api/sales", {
            headers: { "Authorization": token }
        });

        if (!res.ok) {
            if (res.status === 401) {
                alert("Session expired. Please log in again.");
                localStorage.removeItem("sessionToken");
                window.location.href = "/";
            } else {
                alert("Failed to load sales. Status: " + res.status);
            }
            return;
        }

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

        const data = await res.json();
        console.log("API data:", data); // debug

        const sales = data.sales || [];

        // Clear previous rows
        salesTableBody.innerHTML = "";

        if (sales.length === 0) {
            const row = document.createElement("tr");
            row.innerHTML = `<td colspan="9" style="text-align:center;">No sales found</td>`;
            salesTableBody.appendChild(row);
            return;
        }

    sales.forEach(p => {
    const row = document.createElement("tr");
    row.innerHTML = `
        <td>${p.user}</td>
        <td>${p.customer}</td> <!-- show customer name -->
        <td>${p.code}</td>
        <td>${p.brand}</td>
        <td>${p.name}</td>
        <td>${p.quantity}</td>
        <td>€${Number(p.price).toFixed(2)}</td>
        <td>${p.discount}%</td>
        <td>${p.vat_amount}%</td>
        <td>€${Number(p.total_price).toFixed(2)}</td>
        <td>${new Date(p.date).toLocaleString()}</td>
    `;
    salesTableBody.appendChild(row);
});


    } catch (err) {
        console.error("Failed to load sales", err);
        alert("Error connecting to backend");
    }
}

// Call loadSales when the page is loaded
window.addEventListener("DOMContentLoaded", () => {
    console.log("DOM loaded, calling loadSales"); 
    loadSales();
});
