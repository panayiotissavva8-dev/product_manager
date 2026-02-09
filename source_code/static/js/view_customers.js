const logoutBtn = document.getElementById("logoutBtn");
const customersTableBody = document.querySelector("#CustomersTable tbody");

async function loadCustomers(){
    const token = localStorage.getItem("sessionToken");
    if(!token){
        window.location.href = "/";
        return;
    }

    try {
        const res = await fetch("/api/customers", {
            headers: {"Authorization": token}
        });

        if(!res.ok) {
            if(res.status === 401){
                alert("Session expired.Please log in again.");
                localStorage.removeItem("sessionToken");
                window.location.href = "/";
            }else{
                alert("Failed to load customers. Status:" + res.status);
            }
            return;
        }

        const data = await res.json();
        const customers = Array.isArray(data) ? data : data.Customers || [];

        if(!customersTableBody){
            console.error("Table body not found!");
            return;
        }

        customersTableBody.innerHTML = "";

        if(customers.length === 0){
            const row = document.createElement("tr");
            row.innerHTML = `<td colspan="8" style="text-align:center;">No customers found</td>`;
            customersTableBody.appendChild(row);
            return;
        }

        customers.forEach(c => {
            const row = document.createElement("tr");

            row.innerHTML = `
                <td>${c.customer}</td>
                <td>${c.type}</td>
                <td>${c.email}</td>
                <td>${c.phone}</td>
                <td>${c.address}</td>
                <td>${c.postCode}</td>
            `;
            customersTableBody.appendChild(row);
        });

    } catch (err) {
        console.error("Failed to load customers", err);
        alert("Error connecting to backend");
    }
}

logoutBtn.addEventListener("click", async () => {
    const token = localStorage.getItem("sessionToken");
    if(!token){
        window.location.href = "/";
        return;
    }

    try{
        const res = await fetch("/logout", {
            method: "POST",
            headers: { "Authorization": token}
        });

        if(res.ok){
            localStorage.removeItem("sessionToken");
            window.location.href = "/";
        }else{
            alert("Logout failed");
        }
    } catch (err) {
        console.error("Logout error:", err);
    }
});

document.addEventListener("DOMContentLoaded", loadCustomers);
