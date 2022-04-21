// listen for auth status changes
auth.onAuthStateChanged((user) => {
  if (user) {
    console.log("user logged in: " + user.email);
    changeSection(user);

  } else {
    console.log("user logged out");
    changeSection();
  }
});

// login
const loginForm = document.getElementById("login-form");
loginForm.addEventListener("submit", (e) => {
  e.preventDefault();
  console.log("submitted");
  // get user info
  const email = loginForm["input-email"].value;
  const password = loginForm["input-password"].value;
  // log the user in
  auth
    .signInWithEmailAndPassword(email, password)
    .then((cred) => {
      // close the login modal & reset form
      loginForm.reset();
      console.log(email);
    })
    .catch((error) => {
      const errorCode = error.code;
      const errorMessage = error.message;
      toastr.clear();
      NioApp.Toast(errorMessage, "error");
      console.log(errorMessage);
    });
});

// logout
document.getElementById("logout-button").addEventListener("click", (e) => {
  e.preventDefault();
  auth.signOut();
  location.reload();
});
