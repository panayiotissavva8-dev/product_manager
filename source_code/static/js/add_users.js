document.addEventListener("DOMContentLoaded", () => {
const logoutBtn = document.getElementById("logoutBtn");
const addUserForm = document.getElementById("addUserForm");

// --- Logout ---
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

// --- Add user form submit ---
addUserForm.addEventListener("submit", async (e) => {
    e.preventDefault();

    const token = localStorage.getItem("sessionToken");
    if (!token) {
        window.location.href = "/";
        return;
    }

    // --- Grab values from form ---
    const username = document.getElementById("username").value.trim();
    const password = document.getElementById("password").value;
    const confirm_password = document.getElementById("confirmPassword").value;
    const email = document.getElementById("email").value.trim();
    const role = document.getElementById("role").value;
    const termsAccepted = document.getElementById("termsAccepted").checked;

    // --- Validation ---
    if (!username || !password) return alert("Enter username & password");
    if (password !== confirm_password) return alert("Passwords do not match");
    if (!termsAccepted) return alert("You must accept the terms");

    const UserData = { username, password, confirm_password, email, role, termsAccepted };

    try {
        const res = await fetch("/add_sub_user", {
            method: "POST",
            headers: { "Content-Type": "application/json", "Authorization": token },
            body: JSON.stringify(UserData)
        });

        if (res.ok) {
            alert("User added successfully!");
            addUserForm.reset();
        } else if (res.status === 400) {
            const text = await res.text();
            alert("Failed: " + text);
        } else if (res.status === 409) {
            alert("User already exists!");
        } else if (res.status === 401) {
            alert("Session expired. Please log in again.");
            localStorage.removeItem("sessionToken");
            window.location.href = "/";
        } else {
            alert("Failed to add user");
        }
    } catch (err) {
        console.error("Add user error:", err);
        alert("Error adding user");
    }
});
});