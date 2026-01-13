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
            window.location.href = "../html_files/login.html";
        }
    } catch (err) {
        console.error("Error fetching user data:", err);
        alert("Failed to connect to backend.");
    }

    logoutBtn.addEventListener("click", async () => {
        try {
            const response = await fetch("http://localhost:18080/logout", {
                method: "POST",
                headers: { "Content-Type": "application/json" }
            });

            const data = await response.json();

            if (data.status === "success") {
                window.location.href = "source_code/static/login.html";
            } else {
                alert("Logout failed");
            }
        } catch (err) {
            console.error("Error during logout:", err);
        }
    });
});
