const registerBtn = document.getElementById("registerBtn");

registerBtn.addEventListener("click", async () => {
    const username = document.getElementById("username").value.trim();
    const password = document.getElementById("password").value.trim();
    const role = document.getElementById("role").value;

    if (!username || !password) return alert("Enter username & password");

    const response = await fetch("/add_user", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ username, password, role })
    });

    if (response.ok) {
        alert("Registration successful!");
        window.location.href = "/";
    } else {
        const text = await response.text();
        alert("Failed: " + text);
    }
});
