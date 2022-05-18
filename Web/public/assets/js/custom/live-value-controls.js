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







