const signBtn = document.getElementById("sign");
const registerBtn = document.getElementById("register");

signBtn.addEventListener("click", async () => {
    const username = document.getElementById("username").value.trim();
    const password = document.getElementById("password").value.trim();

    if (!username || !password) return alert("Enter username & password");

    try {
        const response = await fetch("/login", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ username, password })
        });

        if (response.ok) {
    const data = await response.json();
    localStorage.setItem("sessionToken", data.token);
    localStorage.setItem("role", data.role);

    if (data.role === "Employee") {
        alert(`Welcome ${data.username}! Role: ${data.role}`);
        window.location.href = "/employee_dashboard";
    } else {
        alert(`Welcome ${data.username}! Role: ${data.role}`);
        window.location.href = "/dashboard";
    }
} else {
    const text = await response.text();
    alert("Login failed: " + text);
}
    } catch (err) {
        console.error("Login error:", err);
        alert("Failed to connect to server");
    }
});

registerBtn.addEventListener("click", () => {
    window.location.href = "/register";
});
