// ===== Elements =====
const backBtn = document.getElementById("backBtn");
const logoutBtn = document.getElementById("logoutBtn"); 
const searchBtn = document.getElementById("searchBtn");
const searchInput = document.getElementById("searchInput");
const saleForm = document.getElementById("saleForm");
const deleteBtn = document.getElementById("deleteBtn");

const inputs = {
    sale_code: document.getElementById("sale_code"),
    user: document.getElementById("user"),
    customer: document.getElementById("customer"),
    code: document.getElementById("code"),
    brand: document.getElementById("brand"),
    name: document.getElementById("name"),
    quantity: document.getElementById("quantity"),
    price: document.getElementById("price"),
    discount: document.getElementById("discount"),
    vat_amount: document.getElementById("vat_amount"),
    total_price: document.getElementById("total_price"),
    date: document.getElementById("date")
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
searchBtn.onclick = searchSale;
searchInput.addEventListener("keydown", e => {
    if (e.key === "Enter") searchSale();
});

async function searchSale() {
    const query = searchInput.value.trim();
    if (!query) return;

    const token = localStorage.getItem("sessionToken");
    if (!token) {
        window.location.href = "/";
        return;
    }

    searchBtn.disabled = true;
    searchBtn.textContent = "Searching...";
    saleForm.classList.add("hidden");

    try {
        const res = await fetch(`/get_sale?query=${encodeURIComponent(query)}`, {
            headers: { "Authorization": token }
        });

        if (!res.ok) {
            alert("Sale not found");
            return;
        }

        const s = await res.json();

        // Fill fields
        inputs.sale_code.value = s.sale_code;
        inputs.user.value = s.user;
        inputs.customer.value = s.customer_id;
        inputs.code.value = s.code;
        inputs.brand.value = s.brand;
        inputs.name.value = s.name;
        inputs.quantity.value = s.quantity;
        inputs.price.value = s.price;
        inputs.discount.value = s.discount;
        inputs.vat_amount.value = s.vat_amount;
        inputs.total_price.value = s.total_price;
        inputs.date.value = s.date;

        // Lock fields
        Object.values(inputs).forEach(i => i.disabled = true);

        saleForm.classList.remove("hidden");
        deleteBtn.disabled = false;
        dirty = false;

    } catch (err) {
        alert("Server error while searching sale");
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

// ===== Delete Product =====
deleteBtn.onclick = async () => {

    const token = localStorage.getItem("sessionToken");
    if (!token) {
        window.location.href = "/";
        return;
    }

    deleteBtn.disabled = true;
    deleteBtn.textContent = "Deleting...";

    const payload = {
        sale_code: Number(inputs.sale_code.value),
        user: inputs.user.value,
        customer: inputs.customer.value,
        code: Number(inputs.code.value),
        brand: inputs.brand.value,
        name: inputs.name.value,
        quantity: Number(inputs.quantity.value),
        price: Number(inputs.price.value),
        discount: Number(inputs.discount.value),
        vat_amount: Number(inputs.vat_amount.value),
        total_price: Number(inputs.total_price.value),
        date: inputs.date.value
    };

    try {
        const res = await fetch("/delete_sale", {
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
                alert("Failed to delete product");
            }
            return;
        }

        // Re-lock fields
        Object.values(inputs).forEach(i => i.disabled = true);
        dirty = false;
        alert("Sale deleted successfully");

    } catch (err) {
        alert("Server error while deleting sale");
        console.error(err);
    } finally {
        deleteBtn.textContent = "Delete Sale";
        deleteBtn.disabled = false;
    }
};
