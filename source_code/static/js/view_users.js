const logoutBtn = document.getElementById("logoutBtn");
const usersTableBody = document.querySelector("#UsersTable tbody");

async function loadUsers(){
    const token = localStorage.getItem("sessionToken");
    if(!token){
        window.location.href = "/";
        return;
    }

    try {
        const res = await fetch("/api/users", {
            headers: {"Authorization": token}
        });

        if(!res.ok) {
            if(res.status === 401){
                alert("Session expired.Please log in again.");
                localStorage.removeItem("sessionToken");
                window.location.href = "/";
            }else{
                alert("Failed to load users. Status:" + res.status);
            }
            return;
        }

        const data = await res.json();
        const users = Array.isArray(data) ? data : data.users || [];

        if(!usersTableBody){
            console.error("Table body not found!");
            return;
        }

        usersTableBody.innerHTML = "";

        if(users.length === 0){
            const row = document.createElement("tr");
            row.innerHTML = `<td colspan="8" style="text-align:center;">No users found</td>`;
            usersTableBody.appendChild(row);
            return;
        }

        users.forEach(u => {
            const row = document.createElement("tr");

            row.innerHTML = `
                <td>${u.username}</td>
                <td>${u.role}</td>
            `;
            usersTableBody.appendChild(row);
        });

    } catch (err) {
        console.error("Failed to load users", err);
        alert("Error connecting to backend");
    }
}

logoutBtn.addEventListener("click", async () => {
    const token = localStorage.getItem("sessionToken");
    if(!token){
        window.location.href = "/";
        return;
    }

    try{
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
    } catch (err) {
        console.error("Logout error:", err);
    }
});

document.addEventListener("DOMContentLoaded", loadUsers);
