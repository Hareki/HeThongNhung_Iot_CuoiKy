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