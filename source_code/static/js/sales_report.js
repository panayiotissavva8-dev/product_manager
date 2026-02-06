async function salesReportGen() {

    const month = document.getElementById("month").value;
    const year = document.getElementById("year").value;

    console.log("sales_report.js loaded");
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

    const res = await fetch(
        `/api/sales_report?month=${month}&year=${year}`,
        {
            headers: { "Authorization": token }
        }
    );

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
        console.log("API data:", data); // debug

        const sales = data.reports || [];

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
               <td>${p.month}</td>
               <td>${p.monthly_sales}</td>
                <td>${p.monthly_units}</td>
               <td>${p.monthly_revenue.toFixed(2)}</td>
               <td>${p.monthly_cost.toFixed(2)}</td>
               <td>${p.monthly_profit.toFixed(2)}</td>`;
            salesTableBody.appendChild(row);
        });
}


// Call salesReportGen when the page is loaded

window.addEventListener("DOMContentLoaded", () => {
    console.log("DOM loaded, calling salesReportGen"); 
    salesReportGen();

    // Add event listeners to month and year dropdowns
   
   const btn = document.getElementById("viewSalesBtn");
    btn.addEventListener("click", () => {
        salesReportGen(); // fetch updated report
    });
});