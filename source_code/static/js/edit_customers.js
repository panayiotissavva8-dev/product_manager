// ===== Elements =====
const backBtn = document.getElementById("backBtn");
const logoutBtn = document.getElementById("logoutBtn"); 
const searchBtn = document.getElementById("searchBtn");
const searchInput = document.getElementById("searchInput");
const customerForm = document.getElementById("customerForm");
const saveBtn = document.getElementById("saveBtn");
let currentCustomerId = null;

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

logoutBtn.onclick = async () => {
    const token = localStorage.getItem("sessionToken");
    if (!token) return window.location.href = "/";

    try {
        const res = await fetch("/logout", { method: "POST", headers: { "Authorization": token }});
        if (res.ok) localStorage.removeItem("sessionToken");
        window.location.href = "/";
    } catch (err) { console.error("Logout error:", err); }
};

// ===== Search Customer =====
searchBtn.onclick = searchCustomer;
searchInput.addEventListener("keydown", e => { if (e.key === "Enter") searchCustomer(); });

async function searchCustomer() {
    const query = searchInput.value.trim();
    if (!query) return;

    const token = localStorage.getItem("sessionToken");
    if (!token) return window.location.href = "/";

    searchBtn.disabled = true;
    searchBtn.textContent = "Searching...";
    customerForm.classList.add("hidden");

    const res = await fetch("/current_user", { headers: { "Authorization": token } });
if (res.ok) {
    const user = await res.text(); // or json depending on your API
    localStorage.setItem("currentUser", user);
}

    try {
        const res = await fetch(`/get_customers?query=${encodeURIComponent(query)}`, {
            headers: { "Authorization": token }
        });

        if (!res.ok) {
            alert("Customer not found");
            return;
        }

        const c = await res.json();

        if (!c.customer_id) {
            alert("Invalid customer data from server");
            return;
        }

        currentCustomerId = c.customer_id;

        // Fill fields
        inputs.customer.value = c.customer;
        inputs.type.value = c.type;
        inputs.email.value = c.email;
        inputs.phone.value = c.phone;
        inputs.address.value = c.address;
        inputs.postCode.value = c.postCode;

        // Lock fields
        Object.values(inputs).forEach(i => i.disabled = true);

        customerForm.classList.remove("hidden");
        saveBtn.disabled = true;
        dirty = false;

    } catch (err) {
        alert("Server error while searching customer");
        console.error(err);
    } finally {
        searchBtn.disabled = false;
        searchBtn.textContent = "Search";
    }
}

// ===== Enable Editing =====
document.querySelectorAll(".field.editable button").forEach(btn => {
    btn.onclick = () => {
        const field = btn.dataset.field;
        inputs[field].disabled = false;
        inputs[field].focus();
    };
});

// ===== Track Changes =====
Object.values(inputs).forEach(input => {
    input.addEventListener("input", () => {
        dirty = true;
        saveBtn.disabled = false;
    });
});

// ===== Save Changes =====
saveBtn.onclick = async () => {
    if (!dirty) return;
    if (!currentCustomerId) {
        alert("No customer selected");
        return;
    }

    const token = localStorage.getItem("sessionToken");
    if (!token) return window.location.href = "/";

    saveBtn.disabled = true;
    saveBtn.textContent = "Saving...";

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

    console.log("Updating customer:", payload);

    try {
        const res = await fetch("/update_customer", {
            method: "POST",
            headers: { "Content-Type": "application/json", "Authorization": token },
            body: JSON.stringify(payload)
        });

        if (!res.ok) {
            if (res.status === 401) {
                alert("Session expired. Please log in again.");
                localStorage.removeItem("sessionToken");
                window.location.href = "/";
            } else if (res.status === 403) {
                alert("Forbidden: You do not have permission to perform this action");
            } else if (res.status === 404) {
                alert("Customer not found");
            } else {
                alert("Failed to update Customer");
            }
            return;
        }

        Object.values(inputs).forEach(i => i.disabled = true);
        dirty = false;
        alert("Customer updated successfully");

    } catch (err) {
        alert("Server error while saving");
        console.error(err);
    } finally {
        saveBtn.textContent = "Save Changes";
        saveBtn.disabled = false;
    }
};
