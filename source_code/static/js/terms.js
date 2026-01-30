// ===== Elements =====
const checkbox = document.getElementById("acceptCheckbox");
const acceptBtn = document.getElementById("acceptBtn");

// ===== Enable button only when checked =====
checkbox.addEventListener("change", () => {
    acceptBtn.disabled = !checkbox.checked;
});

// ===== Accept terms =====
acceptBtn.addEventListener("click", async () => {
    const token = localStorage.getItem("authToken");

    if (!token) {
        // No auth context, redirect to login
        window.location.href = "/login.html";
        return;
    }

    try {
        const res = await fetch("/api/accept-terms", {
            method: "POST",
            headers: {
                "Content-Type": "application/json",
                "Authorization": `Bearer ${token}`
            },
            body: JSON.stringify({
                accepted: true,
                acceptedAt: new Date().toISOString()
            })
        });

        if (!res.ok) {
            throw new Error("Failed to accept terms");
        }

        // Optional local flag to avoid re-showing page
        localStorage.setItem("termsAccepted", "true");

        // Continue to app
        window.location.href = "/dashboard.html";

    } catch (err) {
        alert("Something went wrong while accepting the terms. Try again.");
        console.error(err);
    }
});
