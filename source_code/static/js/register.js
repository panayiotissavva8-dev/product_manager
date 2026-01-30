document.addEventListener("DOMContentLoaded", () => {
const registerBtn = document.getElementById("registerBtn");
const loginBtn = document.getElementById("loginBtn");
const termsCheckbox = document.getElementById("termsCheckbox");

registerBtn.addEventListener("click", async (e) => {
     if (!termsCheckbox.checked) {
        e.preventDefault();
        alert("You must agree to the Terms and Conditions to register.");
        return;
    }

    const username = document.getElementById("username").value.trim();
    const password = document.getElementById("password").value.trim();
    const role = document.getElementById("role").value;

    if (!username || !password) return alert("Enter username & password");

    try {
        const response = await fetch("http://localhost:18080/add_user", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ username, password, role, termsAccepted: true}),
        });

        if (response.ok) {
            alert("Registration successful!");
            window.location.href = "/";
        } else {
            const text = await response.text();
            alert("Failed: " + text);
        }
    } catch (err) {
        console.error("Error registering user:", err);
        alert("Could not connect to server");
    }
});

loginBtn.addEventListener("click", () => {
    window.location.href = "/login";
});
});
