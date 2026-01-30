document.addEventListener("DOMContentLoaded", () => {
    const form = document.querySelector("form");

    if (!form) return;

    form.addEventListener("submit", (e) => {
        const name = document.getElementById("name").value.trim();
        const email = document.getElementById("email").value.trim();
        const message = document.getElementById("message").value.trim();

        if (!name || !email || !message) {
            e.preventDefault();
            alert("Fill in all fields. This is not optional.");
            return;
        }

        if (!validateEmail(email)) {
            e.preventDefault();
            alert("Use a real email address. You know what that means.");
            return;
        }
    });
});

function validateEmail(email) {
    return /^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(email);
}
