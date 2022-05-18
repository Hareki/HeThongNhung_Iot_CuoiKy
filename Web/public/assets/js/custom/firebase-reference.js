firebase.initializeApp(firebaseConfig);
const auth = firebase.auth();
const db = firebase.database();

// VALUES REFERENCES
const tempRef = db.ref("values/temperature");
const humidRef = db.ref("values/humidity");
const gasPPMRef = db.ref("values/gasPPM");
const dustRef = db.ref("values/dustDensity");
const floatReferences = [tempRef, humidRef, gasPPMRef, dustRef];

const onFireRef = db.ref("values/onFire");
const onGasRef = db.ref("values/onGas");


// SETTINGS REFERENCES
const fireAlertRef = db.ref("settings/alert/fire");
const gasAlertRef = db.ref("settings/alert/gas");

const ledBrightnessRef = db.ref("settings/led/brightness");
const ledRGBRef = db.ref("settings/led/RGB");
const ledIsOnRef = db.ref("settings/led/isOn");
const ledIsBlinkingRef = db.ref("settings/led/isBlinking");