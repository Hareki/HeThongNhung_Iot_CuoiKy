function setSensorEnabled(sensorId, enable) {
  var mainContent = document.querySelector(sensorId + "> .main-content");
  var subContent = document.querySelector(sensorId + "> .sub-content");
  var cardFire, titleFire;
  var isFire = sensorId == "#content-fire";

  if (isFire) {
    cardFire = document.getElementById("card-fire");
    titleFire = document.getElementById("title-fire");
  }

  if (enable) {
    mainContent.classList.remove("hidden");
    subContent.innerHTML = "";
    if (isFire) {
      cardFire.classList.add("flame-detected-color");
      titleFire.classList.add("flame-detected-color");
    }
  } else {
    mainContent.classList.add("hidden");
    subContent.innerHTML = `<div class="info">
        <div>
          Cảm biến đã tắt
        </div>
        <div class="center mt-1">
          <div class="sleep-img">&nbsp;</div>
        </div>
      </div>`;

    if (isFire) {
      cardFire.classList.remove("flame-detected-color");
      titleFire.classList.remove("flame-detected-color");
    }
  }
}

function updateSliderValue(){
    var ledValue = document.getElementById('sliderB').value;
    document.getElementById('textSliderValue').innerHTML = ledValue;
}
updateSliderValue();