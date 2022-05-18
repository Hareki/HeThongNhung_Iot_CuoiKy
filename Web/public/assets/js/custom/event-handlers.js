function addAllEventHandler() {
  addValueChangedEventHandler();
  addSettingChangedEventHandler();

  sliderBrightness.addEventListener('change', () => {
    var ledBrightness = document.getElementById('sliderB').value;
    document.getElementById('textSliderValue').innerHTML = ledBrightness;
    ledBrightnessRef.set(parseInt(ledBrightness));
  });

  fireSwitch.addEventListener('change', fireSwitchedEventHandler);
  gasSwitch.addEventListener('change', gasSwitchedEventHandler);
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

function addValueChangedEventHandler() {
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
  
  function addSettingChangedEventHandler() {
    fireAlertRef.on('value', (snap) => {
      fireSwitch.checked = snap.val();
      fireSwitchedEventHandler();
    });
    gasAlertRef.on('value', (snap) => {
      gasSwitch.checked = snap.val();
      gasSwitchedEventHandler();
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
  


  function onFireEventHandler(snap) {
    vOnFire = snap.val();
    fireSwitchedEventHandler();
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

  function fireSwitchedEventHandler(evt) {
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
  
  function gasSwitchedEventHandler() {
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