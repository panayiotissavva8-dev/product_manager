async function salesReportGen(month, year) {
    console.log("Fetching sales for", month, year);
    const token = localStorage.getItem("sessionToken");
    if (!token) {
        window.location.href = "/";
        return;
    }

    const salesTableBody = document.querySelector("#salesTable tbody");
    if (!salesTableBody) {
        console.error("Table tbody not found!");
        return;
    }

    const res = await fetch(`/api/sales_report?month=${month}&year=${year}`, {
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

    const data = await res.json();
    console.log("API data:", data);

    // Clear previous rows
    salesTableBody.innerHTML = "";

    const sales = data.reports || [];
    if (sales.length === 0) {
        const row = document.createElement("tr");
        row.innerHTML = `<td colspan="7" style="text-align:center;">No sales found</td>`;
        salesTableBody.appendChild(row);
        return;
    }

    sales.forEach(p => {
        const row = document.createElement("tr");
        row.innerHTML = `
            <td>${p.month}</td>
            <td>${p.monthly_sales}</td>
            <td>${p.monthly_units}</td>
            <td>${p.monthly_revenue.toFixed(2)}</td>
            <td>${p.monthly_cost.toFixed(2)}</td>
            <td>${p.vat_amount ?? 0}%</td>
            <td>${p.monthly_profit.toFixed(2)}</td>`;
        salesTableBody.appendChild(row);
    });
}

window.addEventListener("DOMContentLoaded", () => {
    const logoutBtn = document.getElementById("logoutBtn");
    const monthSelect = document.getElementById("month");
    const yearSelect = document.getElementById("year");

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

    // Set defaults
    const now = new Date();
    monthSelect.value = String(now.getMonth() + 1).padStart(2, "0");
    yearSelect.value = now.getFullYear();

    // Fetch initially
    salesReportGen(monthSelect.value, yearSelect.value);

    // Auto-fetch whenever month/year changes
    monthSelect.addEventListener("change", () => {
        salesReportGen(String(monthSelect.value).padStart(2, "0"), yearSelect.value);
    });
    yearSelect.addEventListener("change", () => {
        salesReportGen(String(monthSelect.value).padStart(2, "0"), yearSelect.value);
    });
});
