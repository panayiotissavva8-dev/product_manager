// ===== Elements =====
const backBtn = document.getElementById("backBtn");
const logoutBtn = document.getElementById("logoutBtn"); 
const searchBtn = document.getElementById("searchBtn");
const searchInput = document.getElementById("searchInput");
const productForm = document.getElementById("productForm");
const saveBtn = document.getElementById("saveBtn");

const inputs = {
    code: document.getElementById("code"),
    brand: document.getElementById("brand"),
    name: document.getElementById("name"),
    quantity: document.getElementById("quantity"),
    stock_alert: document.getElementById("stock_alert"),
    cost: document.getElementById("cost"),
    price: document.getElementById("price"),
    discount: document.getElementById("discount")
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

// ===== Search Product =====
searchBtn.onclick = searchProduct;
searchInput.addEventListener("keydown", e => {
    if (e.key === "Enter") searchProduct();
});

async function searchProduct() {
    const query = searchInput.value.trim();
    if (!query) return;

    const token = localStorage.getItem("sessionToken");
    if (!token) {
        window.location.href = "/";
        return;
    }

    searchBtn.disabled = true;
    searchBtn.textContent = "Searching...";
    productForm.classList.add("hidden");

    try {
        const res = await fetch(`/get_product?query=${encodeURIComponent(query)}`, {
            headers: { "Authorization": token }
        });

        if (!res.ok) {
            alert("Product not found");
            return;
        }

        const p = await res.json();

        // Fill fields
        inputs.code.value = p.code;
        inputs.brand.value = p.brand;
        inputs.name.value = p.name;
        inputs.quantity.value = p.quantity;
        inputs.stock_alert.value = p.stock_alert;
        inputs.cost.value = p.cost;
        inputs.price.value = p.price;
        inputs.discount.value = p.discount;

        // Lock fields
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
    if (!token) {
        window.location.href = "/";
        return;
    }

    saveBtn.disabled = true;
    saveBtn.textContent = "Saving...";

    const payload = {
        code: Number(inputs.code.value),
        brand: inputs.brand.value,
        name: inputs.name.value,
        quantity: Number(inputs.quantity.value),
        stock_alert: Number(inputs.stock_alert.value),
        cost: Number(inputs.cost.value),
        price: Number(inputs.price.value),
        discount: Number(inputs.discount.value)
    };

    try {
        const res = await fetch("/update_product", {
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
                alert("Failed to update product");
            }
            return;
        }

        // Re-lock fields
        Object.values(inputs).forEach(i => i.disabled = true);
        dirty = false;
        alert("Product updated successfully");

    } catch (err) {
        alert("Server error while saving");
        console.error(err);
    } finally {
        saveBtn.textContent = "Save Changes";
        saveBtn.disabled = false;
    }
};
