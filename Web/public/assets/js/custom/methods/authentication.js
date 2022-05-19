
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

function changeSection(user) {
  if (user) {
    setHidden(loginSection, true);
    setHidden(starterSection, true);
    setHidden(mainSection, false);
    setEmailToUI(user.email);

    loadSettings();
    addAllEventHandler();
  } else {
    setHidden(loginSection, false);
    setHidden(starterSection, true);
    setHidden(mainSection, true);
    setEmailToUI('');
  }
};

function setEmailToUI(email) {
  Array.prototype.forEach.call(userEmailElements, function (element) {
    element.innerText = email;
  });
}

function setHidden(element, hidden) {
  if (hidden) {
    element.classList.add("hidden");
  } else {
    element.classList.remove("hidden");
  }
}

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

function loadSettings() {
  fireAlertRef.once('value')
    .then(function (snap) {
      var fireAlert = snap.val();
      if (fireAlert) {
        fireSwitch.checked = true;

      } else {
        fireSwitch.checked = false;
      }
      fireSwitchedEventHandler();
    });
  gasAlertRef.once('value')
    .then(function (snap) {
      var gasAlert = snap.val();
      if (gasAlert) {
        gasSwitch.checked = true;
      } else {
        gasSwitch.checked = false;
      }
      gasSwitchedEventHandler();
    });
}

// logout
document.getElementById("logout-button").addEventListener("click", (e) => {
  e.preventDefault();
  auth.signOut();
  location.reload();
});
