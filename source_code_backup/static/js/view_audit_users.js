document.addEventListener("DOMContentLoaded", () => {
    const logoutBtn = document.getElementById("logoutBtn");
    const usersTableBody = document.querySelector("#UsersTable tbody");

    async function loadUserAudit(){
        const token = localStorage.getItem("sessionToken");
        if(!token) return window.location.href = "/";

        try {
            const res = await fetch("/api/audit_users", {
                headers: {"Authorization": token}
            });
            if(!res.ok) return alert("Failed to load users. Status:" + res.status);

            const data = await res.json();
            console.log("API response:", data);
            const users = data.Users || [];

            usersTableBody.innerHTML = "";
            if(users.length === 0){
                usersTableBody.innerHTML = `<tr><td colspan="8" style="text-align:center;">No users audit found</td></tr>`;
                return;
            }

            users.forEach(u => {
                usersTableBody.innerHTML += `
                    <tr>
                        <td>${u.audit_id}</td>
                        <td>${u.username}</td>
                        <td>${u.role}</td>
                        <td>${u.timestamp}</td>
                    </tr>
                `;
            });

        } catch (err) {
            console.error(err);
            alert("Error connecting to backend");
        }
    }

    logoutBtn.addEventListener("click", async () => {
        const token = localStorage.getItem("sessionToken");
        if(!token) return window.location.href = "/";

        try {
            const res = await fetch("/logout", {
                method: "POST",
                headers: { "Authorization": token}
            });
            if(res.ok){
                localStorage.removeItem("sessionToken");
                window.location.href = "/";
            }else{
                alert("Logout failed");
            }
        } catch(err) {
            console.error(err);
        }
    });

    loadUserAudit();
});