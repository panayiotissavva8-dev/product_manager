document.addEventListener("DOMContentLoaded", async () => {
    const usernameSpan = document.getElementById("username");
    const roleSpan = document.getElementById("role");
    const logoutBtn = document.getElementById("logoutBtn");

    try {
        // Request user info from backend (you need a GET /current_user endpoint)
        const response = await fetch("http://localhost:18080/current_user", {
            method: "GET",
            headers: { "Content-Type": "application/json" }
        });

        const data = await response.json();

        if (data.status === "success") {
            usernameSpan.textContent = data.username;
            roleSpan.textContent = data.role;
        } else {
            alert("You are not logged in.");
            window.location.href = "/login";
        }
    } catch (err) {
        console.error("Error fetching user data:", err);
        alert("Failed to connect to backend.");
    }

   logoutBtn.addEventListener("click", async () => {
    const token = localStorage.getItem("sessionToken");

    try {
        const response = await fetch("/logout", {
            method: "POST",
            headers: {
                "Content-Type": "application/json",
                "Authorization": token
            }
        });

        if (response.status === 200) {
            localStorage.removeItem("sessionToken");
            window.location.href = "/"; // redirect to login page
        } else {
            alert("Logout failed");
        }
    } catch (err) {
        console.error("Error during logout:", err);
    }
});
});
