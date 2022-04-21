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

// FLOAT REFERENCES
const tempRef = db.ref().child("values/temperature");
const humidRef = db.ref().child("values/humidity");
const gasPPMRef = db.ref().child("values/gasPPM");
const dustRef = db.ref().child("values/dustDensity");

const floatReferences = [tempRef, humidRef, gasPPMRef, dustRef];

// BOOL REFERENCES
const onFireRef = db.ref().child("values/onFire");
const onGasRef = db.ref().child("values/onGas");
const fireAlertRef = db.ref().child("settings/alert/fire");
const gasAlertRef = db.ref().child("settings/alert/gas");

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
// VARIABLES



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

function addFloatChangedEL() {
  var length = floatElements.length;
  for (let i = 0; i < length; i++) {
    floatReferences[i].on("value", (snap) => {
      var value = snap.val().toFixed(1);
      setFloatInfo(floatElements[i], value);
    });
  }
}

function addBoolChangedEL() {
  onFireRef.on('value', (snap) => {
    vOnFire = snap.val();
    var nodes = fireContent.querySelectorAll(".title, .card");
    if (vOnFire) {
      setHidden(document.getElementById("on-fire-true"), false);
      setHidden(document.getElementById("on-fire-false"), true);

      fireSwitchEventHandler();

    } else {
      setHidden(document.getElementById("on-fire-false"), false);
      setHidden(document.getElementById("on-fire-true"), true);

      Array.prototype.forEach.call(nodes, function (element) {
        element.classList.remove("alerting-color");
      });
    }
  })

  onGasRef.on('value', (snap) => {
    vOnGas = snap.val();
    var nodes = document.getElementById("content-gasPPM").querySelectorAll(".title, .card, .amount");
    if (vOnGas) {
      Array.prototype.forEach.call(nodes, function (element) {
        element.classList.add("alerting-color");
      });
    } else {
      Array.prototype.forEach.call(nodes, function (element) {
        element.classList.remove("alerting-color");
      });
    }
  })
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
    obj.color = '#e85347';
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
    obj.color = '#e85347';
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
    obj.color = '#e85347';
  }
  return obj;
}

function sliderBEventHandler() {
  var ledValue = document.getElementById('sliderB').value;
  document.getElementById('textSliderValue').innerHTML = ledValue;
}
function fireSwitchEventHandler(evt) {
  var alert = fireSwitch.checked;
  fireAlertRef.set(alert);
  if (vOnFire) {
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
}
function gasSwitchEventHandler(evt) {
  var alert = gasSwitch.checked;
  gasAlertRef.set(alert);
  if (vOnGas) {
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

}
function addAllEeventListeners() {
  addFloatChangedEL();
  addBoolChangedEL();
  sliderBrightness.addEventListener('change', sliderBEventHandler);
  fireSwitch.addEventListener('change', fireSwitchEventHandler);
  gasSwitch.addEventListener('change', gasSwitchEventHandler);
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
    });
  gasAlertRef.once('value')
    .then(function (snap) {
      var gasAlert = snap.val();
      if (gasAlert) {
        gasSwitch.checked = true;
      } else {
        gasSwitch.checked = false;
      }
    });

}


