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

    if (!username || !password) return alert("Enter username & password");

        if (password !== confirm_password) {
          alert("Passwords do not match.");
           return;
         }

    const UserData = {
        username: document.getElementById("username").value,
        password: document.getElementById("password").value,
        confirm_password: document.getElementById("confirmPassword").value,
        email: document.getElementById("email").value,
        role: document.getElementById("role").value,
        termsAccepted: document.getElementById("termsAccepted").checked
    };

    try {
        const res = await fetch("/add_sub_user", {
            method: "POST",
            headers: { 
                "Content-Type": "application/json",
                "Authorization": token   
            },
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