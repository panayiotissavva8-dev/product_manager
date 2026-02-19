// ===== Elements =====
const backBtn = document.getElementById("backBtn");
const logoutBtn = document.getElementById("logoutBtn"); 
const searchBtn = document.getElementById("searchBtn");
const searchInput = document.getElementById("searchInput");
const productForm = document.getElementById("productForm");
const saveBtn = document.getElementById("saveBtn");

const inputs = {
    customerId: document.getElementById("customerSelect"),
    code: document.getElementById("code"),
    brand: document.getElementById("brand"),
    name: document.getElementById("name"),
    quantity: document.getElementById("quantity"),
    cost: document.getElementById("cost"),
    price: document.getElementById("price"),
    discount: document.getElementById("discount"),
    vat_amount: document.getElementById("vat_amount")
};

let dirty = false;

// ===== Lock all fields by default =====
Object.values(inputs).forEach(i => i.disabled = true);

// ===== Navigation =====
backBtn.onclick = () => window.location.href = "/dashboard";

logoutBtn.addEventListener("click", async () => {
    const token = localStorage.getItem("sessionToken");
    if (!token) { window.location.href = "/"; return; }

    try {
        const res = await fetch("/logout", {
            method: "POST",
            headers: { "Authorization": token }
        });
        if (res.ok) {
            localStorage.removeItem("sessionToken");
            window.location.href = "/";
        } else console.error("Logout failed on server");
    } catch (err) { console.error("Logout error:", err); }
});

// ===== Search Product =====
searchBtn.onclick = searchProduct;
searchInput.addEventListener("keydown", e => { if (e.key === "Enter") searchProduct(); });

async function searchProduct() {
    const query = searchInput.value.trim();
    if (!query) return;

    const token = localStorage.getItem("sessionToken");
    if (!token) { window.location.href = "/"; return; }

    searchBtn.disabled = true;
    searchBtn.textContent = "Searching...";
    productForm.classList.add("hidden");

    try {
        const res = await fetch(`/get_product?query=${encodeURIComponent(query)}`, {
            headers: { "Authorization": token }
        });

        if (!res.ok) { alert("Product not found"); return; }

        const p = await res.json();

        // Fill fields
        inputs.customerId.value = p.customerId || "";
        inputs.code.value = p.code;
        inputs.brand.value = p.brand;
        inputs.name.value = p.name;
        inputs.cost.value = p.cost;
        inputs.price.value = p.price;
        inputs.discount.value = p.discount;
        inputs.vat_amount.value = p.vat_amount;

        // Keep fields locked until edit
        Object.values(inputs).forEach(i => i.disabled = true);

        productForm.classList.remove("hidden");
        saveBtn.disabled = true;
        dirty = false;

    } catch (err) {
        alert("Server error while searching product");
        console.error(err);
    } finally {
        searchBtn.disabled = false;
        searchBtn.textContent = "Search";
    }
}

// ===== Load Customers into Dropdown =====
async function loadCustomers() {
    const token = localStorage.getItem("sessionToken");
    if (!token) return;

    try {
        const res = await fetch("/api/customers", { headers: { Authorization: token } });
        if (!res.ok) { console.error("Failed to load customers"); return; }

        const data = await res.json();
        const customers = data.Customers || [];
        const select = document.getElementById("customerSelect");
        select.innerHTML = `<option value="">Select customer</option>`;
        customers.forEach(c => {
            const opt = document.createElement("option");
            opt.value = c.customer_id;
            opt.textContent = c.customer;
            select.appendChild(opt);
        });

        select.disabled = true; // lock dropdown initially
    } catch (err) { console.error("Error loading customers", err); }
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

    const token = localStorage.getItem("sessionToken");
    if (!token) { window.location.href = "/"; return; }

    saveBtn.disabled = true;
    saveBtn.textContent = "Saving...";

    const payload = {
        customer_id: inputs.customerId.value,
        code: Number(inputs.code.value),
        brand: inputs.brand.value,
        name: inputs.name.value,
        quantity: Number(inputs.quantity.value),
        cost: Number(inputs.cost.value),
        price: Number(inputs.price.value),
        discount: Number(inputs.discount.value),
        vat_amount: Number(inputs.vat_amount.value)
    };

    try {
        const res = await fetch("/add_sale", {
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
            } else if (res.status === 400) {
                alert("Insufficient stock for this sale. Adjust quantity and try again.");
            } else if (res.status === 403) {
                alert("Sale added. Warning: Stock below alert level!");
            } else {
                alert("Failed to add sale. Check input and try again.");   
            }
            return;
        }

        // Re-lock fields after saving
        Object.values(inputs).forEach(i => i.disabled = true);
        dirty = false;
        alert("Sale added successfully");

    } catch (err) {
        alert("Server error while saving");
        console.error(err);
    } finally {
        saveBtn.textContent = "Save Changes";
        saveBtn.disabled = false;
    }
};

// ===== Init =====
window.addEventListener("DOMContentLoaded", loadCustomers);
