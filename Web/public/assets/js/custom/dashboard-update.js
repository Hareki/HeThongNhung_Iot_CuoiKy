const loginSectionElement = document.getElementById("login-section");
const userEmailElement = document.getElementById("user-email");
const contentElement = document.getElementById("content-sign-in");

// Elements for sensor readings
const tempElement = document.getElementById("value-temp");
const humidElement = document.getElementById("value-humid");
const gasPPMElement = document.getElementById("value-gasPPM");
const dustElement = document.getElementById("value-dust");
// MANAGE LOGIN/LOGOUT UI
const setupUI = (user) => {
  if (user) {
    //toggle UI elements
    loginSectionElement.classList.add("hidden");
    contentElement.classList.remove("hidden");
    userEmailElement.innerHTML = user.email;

    // Database references
    var tempRef = firebase.database().ref().child("sensor/temperature");
    var humidRef = firebase.database().ref().child("sensor/humidity");
    var gasPPMRef = firebase.database().ref().child("sensor/gasPPM");
    var dustRef = firebase.database().ref().child("sensor/dustDensity");

   // Update page with new readings
    tempRef.on("value", (snap) => {
      console.log("temp update" + snap.val());
      tempElement.innerText = snap.val().toFixed(1);
    });

    humidRef.on("value", (snap) => {
      console.log("humid update" + snap.val());
      humidElement.innerText = snap.val().toFixed(1);
    });

    gasPPMRef.on("value", (snap) => {
      console.log("gasPPM update" + snap.val());
      gasPPMElement.innerText = snap.val().toFixed(1);
    });

    dustRef.on("value", (snap) => {
      console.log("dust update" + snap.val());
      dustElement.innerText = snap.val().toFixed(1);
    });

    //tempRef.set(444);
    // get user UID to get data from database
    var uid = user.uid;
    console.log("user id: " + uid);

    // if user is logged out
  } else {
    loginSectionElement.classList.remove("hidden");
    contentElement.classList.add("hidden");
    userEmailElement.innerHTML = "";
  }
};
