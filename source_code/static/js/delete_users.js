// ===== Elements =====
const backBtn = document.getElementById("backBtn");
const logoutBtn = document.getElementById("logoutBtn"); 
const searchBtn = document.getElementById("searchBtn");
const searchInput = document.getElementById("searchInput");
const userForm = document.getElementById("userForm");
const deleteBtn = document.getElementById("deleteBtn");

const inputs = {
    username: document.getElementById("username"),
    role: document.getElementById("role"),
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

// ===== Search User =====
searchBtn.onclick = searchUser;
searchInput.addEventListener("keydown", e => {
    if (e.key === "Enter") searchUser();
});

async function searchUser() {
    const query = searchInput.value.trim();
    if (!query) return;

    const token = localStorage.getItem("sessionToken");
    if (!token) {
        window.location.href = "/";
        return;
    }

    searchBtn.disabled = true;
    searchBtn.textContent = "Searching...";
    userForm.classList.add("hidden");

    try {
        const res = await fetch(`/get_users?query=${encodeURIComponent(query)}`, {
            headers: { "Authorization": token }
        });

        if (!res.ok) {
            alert("User not found");
            return;
        }

        const u = await res.json();
        
        currentUserId = u.user_id;
        // Fill fields
        inputs.username.value = u.username;
        inputs.role.value = u.role;

        // Lock fields
        Object.values(inputs).forEach(i => i.disabled = true);

        userForm.classList.remove("hidden");
        deleteBtn.disabled = false;
        dirty = false;

    } catch (err) {
        alert("Server error while searching user");
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

// ===== Delete User =====
deleteBtn.onclick = async () => {

    const token = localStorage.getItem("sessionToken");
    if (!token) {
        window.location.href = "/";
        return;
    }

    deleteBtn.disabled = true;
    deleteBtn.textContent = "Deleting...";

    const payload = {
         user_id: currentUserId,
        owner: localStorage.getItem("currentUser") || "",
        username: inputs.username.value,
        role: inputs.role.value,
    };

    try {
        const res = await fetch("/delete_user", {
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
                alert("Failed to delete User");
            }
            return;
        }

        // Re-lock fields
        Object.values(inputs).forEach(i => i.disabled = true);
        dirty = false;
        alert("User deleted successfully");

    } catch (err) {
        alert("Server error while deleting User");
        console.error(err);
    } finally {
        deleteBtn.textContent = "Delete User";
        deleteBtn.disabled = false;
    }
};
