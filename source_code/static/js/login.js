const signBtn = document.getElementById("sign");
const registerBtn = document.getElementById("register");

signBtn.addEventListener("click", async () => {
    const username = document.getElementById("username").value.trim();
    const password = document.getElementById("password").value.trim();

    if (!username || !password) return alert("Enter username & password");

    const response = await fetch("/login", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ username, password })
    });

    if (response.ok) {
        const data = await response.json();
        alert(`Welcome ${data.username}! Role: ${data.role}`);
        // Redirect to dashboard or main page
        window.location.href = "/dashboard";
    } else {
        alert("Invalid credentials");
    }
});

registerBtn.addEventListener("click", () => {
    window.location.href = "/register";
});
