//SECTIONS
const loginSection = document.getElementById("login-section");
const mainSection = document.getElementById("main-section");
const starterSection = document.getElementById("starter-section");

const userEmailElement = document.getElementById("user-email");
const userEmailElements = document.getElementsByClassName('user-email');

// FLOAT ELEMENTS
const tempElement = document.getElementById("value-temperature");
const humidElement = document.getElementById("value-humidity");
const gasPPMElement = document.getElementById("value-gasPPM");
const dustElement = document.getElementById("value-dust-density");
const floatElements = [tempElement, humidElement, gasPPMElement, dustElement];

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

// CONTENS
const fireContent = document.getElementById('content-fire');
const gasPPMContent = document.getElementById('content-gasPPM');
const temperatureContent = document.getElementById('content-temperature');
const humidityContent = document.getElementById('content-humidity');
const ledContent = document.getElementById('content-led');

//OTHER ELEMENTS
const gasSwitch = document.getElementById('gas-switch');
gasSwitch.contentId = 'content-gasPPM';

const fireSwitch = document.getElementById('fire-switch');
fireSwitch.contentId = 'content-fire';

const sliderBrightness = document.getElementById('sliderB');
const fireIMG = document.getElementById("fire-img")
const buttonSelectColor = document.getElementById('button-select-color');
const onSwitch = document.getElementById('on-switch');
const blinkingSwitch = document.getElementById('blinking-switch');
const hexInput = document.getElementById('hex-input');
const sliderElement = document.getElementById('sliderB');

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

var testFire;
function changeSection(user) {
  if (user) {
    setHidden(loginSection, true);
    setHidden(starterSection, true);
    setHidden(mainSection, false);
    setEmailToUI(user.email);

    loadSettings();
    addAllEeventListeners();
  } else {
    setHidden(loginSection, false);
    setHidden(starterSection, true);
    setHidden(mainSection, true);
    setEmailToUI('');
  }
};


function addValueChangedEventListener() {
  var length = floatElements.length;
  for (let i = 0; i < length; i++) {
    floatReferences[i].on("value", (snap) => {
      var value = snap.val().toFixed(1);
      setFloatInfo(floatElements[i], value);
    });
  }

  onFireRef.on('value', onFireEventHandler)
  onGasRef.on('value', onGasEventHandler)
}

function addSettingChangedEventListener() {
  fireAlertRef.on('value', (snap) => {
    fireSwitch.checked = snap.val();
    fireSwitchEventHandler();
  });
  gasAlertRef.on('value', (snap) => {
    gasSwitch.checked = snap.val();
    gasSwitchEventHandler();
  });

  ledBrightnessRef.on('value', (snap) => {
    var value = snap.val();
    sliderElement.setAttribute('value', value);
    document.getElementById('textSliderValue').innerHTML = value;
  });

  ledRGBRef.on('value', (snap) => {
    var value = snap.val();
    hexInput.value = value.toUpperCase();
    hexInput.style.backgroundColor = '#' + value.toString();
  });

  ledIsOnRef.on('value', (snap) => {
    onSwitch.checked = snap.val();
  });

  ledIsBlinkingRef.on('value', (snap) => {
    blinkingSwitch.checked = snap.val();
  });
}

function setFloatInfo(element, value) {
  element.innerText = value;

  var obj;
  var parentId = 'content-';
  switch (element.id) {
    case 'value-temperature':
      obj = getTemperatureInfo(value);
      vTemperature = value;
      parentId += 'temperature';
      break;
    case 'value-humidity':
      obj = getHumidityInfo(value);
      vHumidity = value;
      parentId += 'humidity';
      break;
    case 'value-dust-density':
      obj = getDustDensityInfo(value);
      vDustDensity = value;
      parentId += 'dust-density';
      break;
    case 'value-gasPPM':
      obj = getGasInfo(value);
      vGasPPM = value;
      parentId += 'gasPPM';
      break;
  }

  var info2Node = document.getElementById(parentId).querySelector('.info2');
  info2Node.innerText = obj.message;
  info2Node.style.background = obj.color;
}

function getDustDensityInfo(value) {
  var obj = {};
  if (value <= 12) {
    obj.message = 'Chất lượng không khí tốt';
    obj.color = '#3ae83a';
  } else if (value <= 35.4) {
    obj.message = 'Chất lượng không khí trung bình';
    obj.color = '#fff200';
  }
  else if (value <= 150.4) {
    obj.message = 'Chất lượng không khí kém';
    obj.color = '#fc7f03';
  }
  else if (value <= 250.4) {
    obj.message = 'Chất lượng không khí xấu';
    obj.color = '#FF1F00';
  }
  else if (value <= 350.4) {
    obj.message = 'Chất lượng không khí rất xấu';
    obj.color = '#816bff';
  }
  else {
    obj.message = 'Chất lượng không khí nguy hại';
    obj.color = '#6B2504';
  }
  return obj;
}
function getTemperatureInfo(value) {
  var obj = {};
  if (value <= 20) {
    obj.message = 'Nhiệt độ lạnh';
    obj.color = '#93b3ff';
  } else if (value <= 27) {
    obj.message = 'Nhiệt độ mát mẻ';
    obj.color = '#3ae83a';
  }
  else if (value <= 33) {
    obj.message = 'Nhiệt độ trung bình';
    obj.color = '#ffa857';
  } else {
    obj.message = 'Nhiệt độ nóng';
    obj.color = '#FF1F00';
  }
  return obj;
}
function getHumidityInfo(value) {
  var obj = {};
  if (value <= 40) {
    obj.message = 'Độ ẩm thấp';
    obj.color = '#FF1F00';
  } else if (value <= 65) {
    obj.message = 'Độ ẩm lý tưởng';
    obj.color = '#3ae83a';
  } else {
    obj.message = 'Độ ẩm cao';
    obj.color = '#0832F1';
  }
  return obj;
}
function getGasInfo(value) {
  var obj = {};
  if (value <= 500) {
    obj.message = 'Nồng độ ga ở mức an toàn';
    obj.color = '#3ae83a';
  } else if (value <= 1000) {
    obj.message = 'Nồng độ ga ở mức trung bình';
    obj.color = '#fc7f03';
  } else {
    obj.message = 'Nồng độ ga cao bất thường';
    obj.color = '#FF1F00';
  }
  return obj;
}

function onFireEventHandler(snap) {
  vOnFire = snap.val();
  fireSwitchEventHandler();
  if (vOnFire) {
    setHidden(document.getElementById("on-fire-true"), false);
    setHidden(document.getElementById("on-fire-false"), true);
    if (fireSwitch.checked) {
      setFireAlertUI(true);
    }
    return;
  } else {
    setHidden(document.getElementById("on-fire-false"), false);
    setHidden(document.getElementById("on-fire-true"), true);
    setFireAlertUI(false);
  }
}

function onGasEventHandler(snap) {
  vOnGas = snap.val();
  if (vOnGas && gasSwitch.checked) {
    setGasAlertUI(true);
  }
  else {
    setGasAlertUI(false);
  }
}

function fireSwitchEventHandler(evt) {
  var alert = fireSwitch.checked;
  fireAlertRef.set(alert);
  if (vOnFire) {
    if (alert) {
      setFireAlertUI(true);
    } else {
      setFireAlertUI(false);
    }
  }
}

function gasSwitchEventHandler() {
  var alert = gasSwitch.checked;
  gasAlertRef.set(alert);
  if (vOnGas) {
    if (alert) {
      setGasAlertUI(true);
    } else {
      setGasAlertUI(false);
    }
  }
}

function setGasAlertUI(alert) {
  var nodes = gasPPMContent.querySelectorAll('.card, .title, .amount');
  if (alert) {
    Array.prototype.forEach.call(nodes, element => {
      element.classList.add('alerting-color');
    });
  } else {
    Array.prototype.forEach.call(nodes, element => {
      element.classList.remove('alerting-color');
    });
  }
}

function setFireAlertUI(alert) {
  var nodes = fireContent.querySelectorAll('.card, .title');
  if (alert) {
    fireIMG.classList.replace('fire-detected-img-red', 'fire-detected-img');
    Array.prototype.forEach.call(nodes, element => {
      element.classList.add('alerting-color');
    });
  } else {
    fireIMG.classList.replace('fire-detected-img', 'fire-detected-img-red');
    Array.prototype.forEach.call(nodes, element => {
      element.classList.remove('alerting-color');
    });
  }
}

function addAllEeventListeners() {
  addValueChangedEventListener();
  addSettingChangedEventListener();

  sliderBrightness.addEventListener('change', () => {
    var ledBrightness = document.getElementById('sliderB').value;
    document.getElementById('textSliderValue').innerHTML = ledBrightness;
    ledBrightnessRef.set(parseInt(ledBrightness));
  });

  fireSwitch.addEventListener('change', fireSwitchEventHandler);
  gasSwitch.addEventListener('change', gasSwitchEventHandler);
  onSwitch.addEventListener('change', () => {
    ledIsOnRef.set(onSwitch.checked);
  });
  blinkingSwitch.addEventListener('change', () => {
    ledIsBlinkingRef.set(blinkingSwitch.checked);
  });
  buttonSelectColor.addEventListener('click', () => {
    ledRGBRef.set(hexInput.value.toUpperCase());
  });
}

function loadSettings() {
  fireAlertRef.once('value')
    .then(function (snap) {
      var fireAlert = snap.val();
      if (fireAlert) {
        fireSwitch.checked = true;

      } else {
        fireSwitch.checked = false;
      }
      fireSwitchEventHandler();
    });
  gasAlertRef.once('value')
    .then(function (snap) {
      var gasAlert = snap.val();
      if (gasAlert) {
        gasSwitch.checked = true;
      } else {
        gasSwitch.checked = false;
      }
      gasSwitchEventHandler();
    });

}


