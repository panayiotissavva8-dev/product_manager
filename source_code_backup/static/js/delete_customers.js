// ===== Elements =====
const backBtn = document.getElementById("backBtn");
const logoutBtn = document.getElementById("logoutBtn"); 
const searchBtn = document.getElementById("searchBtn");
const searchInput = document.getElementById("searchInput");
const customerForm = document.getElementById("customerForm");
const deleteBtn = document.getElementById("deleteBtn");

const inputs = {
    customer: document.getElementById("customer"),
    type: document.getElementById("type"),
    email: document.getElementById("email"),
    phone: document.getElementById("phone"),
    address: document.getElementById("address"),
    postCode: document.getElementById("postCode")
};

let dirty = false;

// ===== Navigation =====
backBtn.onclick = () => window.location.href = "/dashboard";

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

// ===== Search Customer =====
searchBtn.onclick = searchCustomer;
searchInput.addEventListener("keydown", e => {
    if (e.key === "Enter") searchCustomer();
});

async function searchCustomer() {
    const query = searchInput.value.trim();
    if (!query) return;

    const token = localStorage.getItem("sessionToken");
    if (!token) {
        window.location.href = "/";
        return;
    }

    searchBtn.disabled = true;
    searchBtn.textContent = "Searching...";
    customerForm.classList.add("hidden");

    try {
        const res = await fetch(`/get_customers?query=${encodeURIComponent(query)}`, {
            headers: { "Authorization": token }
        });

        if (!res.ok) {
            alert("Customer not found");
            return;
        }

        const c = await res.json();
        
        currentCustomerId = c.customer_id;
        // Fill fields
        inputs.customer.value = c.customer;
        inputs.type.value = c.type;
        inputs.email.value = c.email;
        inputs.phone.value = c.phone;
        inputs.address.value = c.address;
        inputs.postCode.value = c.post_code;

        // Lock fields
        Object.values(inputs).forEach(i => i.disabled = true);

        customerForm.classList.remove("hidden");
        deleteBtn.disabled = false;
        dirty = false;

    } catch (err) {
        alert("Server error while searching customer");
        console.error(err);
    } finally {
        searchBtn.disabled = false;
        searchBtn.textContent = "Search";
    }
}


// ===== Track Changes =====
Object.values(inputs).forEach(input => {
    input.addEventListener("input", () => {
        dirty = true;
        deleteBtn.disabled = false;
    });
});

// ===== Delete Customer =====
deleteBtn.onclick = async () => {

    const token = localStorage.getItem("sessionToken");
    if (!token) {
        window.location.href = "/";
        return;
    }

    deleteBtn.disabled = true;
    deleteBtn.textContent = "Deleting...";

    const payload = {
         customer_id: currentCustomerId,
        owner: localStorage.getItem("currentUser") || "",
        customer: inputs.customer.value,
        type: inputs.type.value,
        email: inputs.email.value,
        phone: String(inputs.phone.value),
        address: inputs.address.value,
        postCode: String(inputs.postCode.value)
    };

    try {
        const res = await fetch("/delete_customer", {
            method: "POST",
            headers: { 
                "Content-Type": "application/json",
                "Authorization": token
            },
            body: JSON.stringify(payload)
        });

        if (!res.ok) {
            if (res.status === 401) {
                alert("Session expired. Please log in again.");
                localStorage.removeItem("sessionToken");
                window.location.href = "/";
            } else {
                alert("Failed to delete customer");
            }
            return;
        }

        // Re-lock fields
        Object.values(inputs).forEach(i => i.disabled = true);
        dirty = false;
        alert("Customer deleted successfully");

    } catch (err) {
        alert("Server error while deleting customer");
        console.error(err);
    } finally {
        deleteBtn.textContent = "Delete Customer";
        deleteBtn.disabled = false;
    }
};
