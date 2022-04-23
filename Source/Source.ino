#include <FirebaseESP32.h>
#include <MQUnifiedsensor.h>
#include <WiFi.h>
#include <addons/TokenHelper.h>
#include <esp_task_wdt.h>

#include "DHT.h"

/*========= DEFINE PORTS =========*/
#define ANALOG_GAS 32
#define ANALOG_FIRE 33
#define ANALOG_TEHU 19  // TEMPERATURE + HUMID = TEHU
#define ANALOG_WHISTLE 12

#define DUST_1 "VCC"  // LED
#define DUST_2 "GND"  // LED
#define DUST_3 18     // LED
#define DUST_4 "GND"
#define DUST_5 35
#define DUST_6 "VCC"

#define LED_RED 23
#define LED_GREEN 22
#define LED_BLUE 21

#define LED_RED_PWM 0
#define LED_GREEN_PWM 1
#define LED_BLUE_PWM 2
/*========= END OF DEFINE PORTS =========*/

/*========= DEFINE CONNECTION INFORMATION =========*/
#define DATABASE_URL "iot-project-7486d-default-rtdb.asia-southeast1.firebasedatabase.app"
#define SECRECT_KEY "t9gujxIcFO6k3k6V2LX71UO89UMReJ8rVghlMuW5"
#define API_KEY "AIzaSyDExcDgLNEP2xf2hxtQS-nZLdFBtK-2ZzM"

#define USER_EMAIL "minhtu1392000@gmail.com"
#define USER_PASSWORD "123456789"

#define WIFI_SSID "Asura"
#define WIFI_PASSWORD "30121995@sura"
/*========= END OF DEFINE CONNECTION INFORMATION =========*/

/*========= DEFINE SENSORS =========*/
#define R0 (0.58)
#define Board ("ESP-32")
#define Type ("MQ-2")
#define Voltage_Resolution (5)
#define ADC_Bit_Resolution (12)
#define RatioMQ2CleanAir (9.83)  // RS / R0 = 9.83 ppm
MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, ANALOG_GAS, Type);

#define DHTTYPE DHT22
DHT dht(ANALOG_TEHU, DHTTYPE);
/*========= END OF DEFINE SENSORS =========*/

/*========= DEFINE COMMON VARIABLES =========*/
#define GASPPM_GREATER_THRESHOLD 1000
#define FIRE_LOWER_THRESHOLD 330
const TickType_t TASK_DELAY_TIME = 1000 / portTICK_PERIOD_MS;
const TickType_t CHECKING_GAS_FIRE_TIME = 250 / portTICK_PERIOD_MS;

#define TASK_DELAY_TIME_2 1000
#define CHECKING_GAS_FIRE_TIME_2 250

#define LED_FREQUENCY 5000
#define LED_RESOLUTION 8

float gasPPM = 0;     // biến giao tiếp giữa gasAlertTask() và sendGasData()
bool onFire = false;  // biến giao tiếp giữa gasAlertTask() và fireAlertTask()

/*========= END OF DEFINE COMMON VARIABLES =========*/

/*========= DEFINE FIREBASE VARIABLES =========*/
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

const String SENSOR_BASE_PATH = "values/";
const String ON_BASE_PATH = "values/";

const String SETTING_BASE_PATH = "settings/";
const String ALERT_BASE_PATH = "settings/alert/";
const String LED_BASE_PATH = "settings/led/";
/*========= END OF DEFINE FIREBASE VARIABLES =========*/

/*========= DEFINE SEMAPHORE =========*/
SemaphoreHandle_t semaphore = NULL;
void initSemaphore() {
  semaphore = xSemaphoreCreateBinary();
  if (semaphore != NULL) {
    xSemaphoreGive(semaphore);
  }
}
/*========= END OF DEFINE SEMAPHORE =========*/

/*========= SETTINGS UPDATER REGION =========*/

// Setting variables
bool gasAlert;
bool fireAlert;

int ledBrightness = 50;
bool ledIsOn = false;
bool ledIsBlinking = false;
String ledRGBHex = "FFFFFF";
int ledRedValue = 255;
int ledGreenValue = 255;
int ledBlueValue = 255;

FirebaseData settingStream;
bool dataIsNode(StreamData data, String node) { return strstr(data.dataPath().c_str(), node.c_str()); }

bool dataIsInt(StreamData data) { return data.dataTypeEnum() == fb_esp_rtdb_data_type_integer; }
bool dataIsBool(StreamData data) { return data.dataTypeEnum() == fb_esp_rtdb_data_type_boolean; }
bool dataIsString(StreamData data) { return data.dataTypeEnum() == fb_esp_rtdb_data_type_string; }
bool dataIsNull(StreamData data) { return data.dataTypeEnum() == fb_esp_rtdb_data_type_null; }
bool dataIsJson(StreamData data) { return data.dataTypeEnum() == fb_esp_rtdb_data_type_json; }

//void loadSettings() {
//  Serial.println("Loading settings...");
//  if (!Firebase.getInt(fbdo, LED_BASE_PATH + "brightness")) Serial.println("fail BR");
//  ledBrightness = fbdo.to<int>();
//  setBrightness(ledBrightness);
//  Serial.println(ledBrightness);
//
//  if (!Firebase.getBool(fbdo, LED_BASE_PATH + "isBlinking")) Serial.println("fail BR");
//  ledIsBlinking = fbdo.to<bool>();
//  setBlinking(ledIsBlinking);
//  Serial.println(ledIsBlinking);
//
//  if (!Firebase.getBool(fbdo, LED_BASE_PATH + "isOn")) Serial.println("fail BR");
//  ledIsOn = fbdo.to<bool>();
//  setPower(ledIsOn);
//  Serial.println(ledIsOn);
//
//  if (!Firebase.getString(fbdo, LED_BASE_PATH + "RGB")) Serial.println("fail BR");
//  ledRGBHex = fbdo.to<String>();
//  setRGB(ledRGBHex);
//  Serial.println(ledRGBHex);
//
//  if (!Firebase.getBool(fbdo, ALERT_BASE_PATH + "gas")) Serial.println("fail BR");
//  gasAlert = fbdo.to<bool>();
//  Serial.println(gasAlert);
//
//  if (!Firebase.getBool(fbdo, ALERT_BASE_PATH + "fire")) Serial.println("fail BR");
//  fireAlert = fbdo.to<bool>();
//  Serial.println(fireAlert);
//
//  Serial.println("done loading settings");
//}

void loadSettings() {
  Serial.println("Loading settings...");
  if (!Firebase.getInt(fbdo, "settings/led/brightness")) Serial.println("fail BR");
  ledBrightness = fbdo.to<int>();
//  setBrightness(ledBrightness);
  Serial.println(ledBrightness);

  if (!Firebase.getBool(fbdo, "settings/led/isBlinking")) Serial.println("fail BR");
  ledIsBlinking = fbdo.to<bool>();
 // setBlinking(ledIsBlinking);
  Serial.println(ledIsBlinking);

  if (!Firebase.getBool(fbdo, "settings/led/isOn")) Serial.println("fail BR");
  ledIsOn = fbdo.to<bool>();
//  setPower(ledIsOn);
  Serial.println(ledIsOn);

  if (!Firebase.getString(fbdo, "settings/led/RGB")) Serial.println("fail BR");
  ledRGBHex = fbdo.to<String>();
//  setRGB(ledRGBHex);
  Serial.println(ledRGBHex);

  if (!Firebase.getBool(fbdo, "settings/alert/gas")) Serial.println("fail BR");
 // gasAlert = fbdo.to<bool>();
  Serial.println(gasAlert);

  if (!Firebase.getBool(fbdo, "settings/alert/fire")) Serial.println("fail BR");
 // fireAlert = fbdo.to<bool>();
  Serial.println(fireAlert);

  Serial.println("done loading settings");
}

void timeOutCallBack(bool timeout) {
  if (timeout) {
    Serial.println("ERROR - Quá thời hạn callback(timeout) \n");
  }
  if (!settingStream.httpConnected()) {
    Serial.printf("ERROR - Lỗi callback: %d, lý do: %s\n\n", settingStream.httpCode(),
                  settingStream.errorReason().c_str());
  }
}

void settingChangedCallBack(StreamData data) {
  if (dataIsJson(data)) return;

  if (dataIsBool(data)) {
    if (dataIsNode(data, "isBlinking")) {
      ledIsBlinking = data.to<bool>();
      setBlinking(ledIsBlinking);
    } else if (dataIsNode(data, "isOn")) {
      ledIsOn = data.to<bool>();
      setPower(ledIsOn);
    } else if (dataIsNode(data, "gas")) {
      gasAlert = data.to<bool>();
    } else if (dataIsNode(data, "fire")) {
      fireAlert = data.to<bool>();
    }
  } else if (dataIsString(data) && dataIsNode(data, "RGB")) {
    ledRGBHex = data.to<String>();
    setRGB(ledRGBHex);
  } else if (dataIsInt(data) && dataIsNode(data, "brightness")) {
    ledBrightness = data.to<int>();
    setBrightness(ledBrightness);
  } else {
    Serial.println("ERROR - Failed to update settings");
  }
}

bool beginSettingChangesListener() {
  if (!Firebase.beginStream(settingStream, SETTING_BASE_PATH)) {
    Serial.printf("ERROR Setting stream begin error, %s\n\n", settingStream.errorReason().c_str());
    return false;
  }
  Firebase.setStreamCallback(settingStream, settingChangedCallBack, timeOutCallBack);
  return true;
}
/*=========END OF SETTINGS UPDATER REGION=========*/

/*========= LED CONTROL MEDTHODS =========*/
void setBrightness(int brightness) {
  double percent = (double)brightness / (double)100;
  int realLedRedValue = round((double)ledRedValue * percent);
  int realLedGreenValue = round((double)ledGreenValue * percent);
  int realLedBlueValue = round((double)ledBlueValue * percent);

  ledcWrite(LED_RED_PWM, realLedRedValue);
  ledcWrite(LED_GREEN_PWM, realLedGreenValue);
  ledcWrite(LED_BLUE_PWM, realLedBlueValue);
}

void setPower(bool isOn) {
  if (isOn) {
    setBrightness(ledBrightness);
  } else {
    setBrightness(0);
  }
}

/*--blinking--*/
bool ledState = true;
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// Hàm ngắt xử lý nhấp nháy
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);  // vào chế độ tránh xung đột

  if (ledState) {
    ledState = false;
    setBrightness(0);
  } else {
    ledState = true;
    setBrightness(ledBrightness);
  }

  portEXIT_CRITICAL_ISR(&timerMux);  // thoát
}
void setBlinking(bool isBlinking) {
  if (isBlinking) {
    timerAlarmEnable(timer);
  } else {
    timerAlarmDisable(timer);
  }
}
/*--blinking--*/

void convertHexToRGB(String hexString) {
  int hexValue = (int)strtol(hexString.c_str(), NULL, 16);
  ledRedValue = ((hexValue >> 16) & 0xFF) / 255.0;
  ledGreenValue = ((hexValue >> 8) & 0xFF) / 255.0;
  ledBlueValue = ((hexValue)&0xFF) / 255.0;
}
void setRGB(String hexString) {
  convertHexToRGB(hexString);
  setBrightness(ledBrightness);
}

/*========= END OF LED CONTROL MEDTHODS =========*/

/*========= CONNECTION MEDTHODS =========*/
void connectToWifi() {
  Serial.print("Đang kết nối tới WiFi ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Kết nối vào WiFi thành công!");
  Serial.println("Địa chỉ IP: ");
  Serial.println(WiFi.localIP());
}

void connectToFirebase() {
  Serial.println("\nĐang kết nối tới Firebase, vui lòng chờ...");
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

//  auth.user.email = USER_EMAIL;
//  auth.user.password = USER_PASSWORD;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  config.signer.tokens.legacy_token = SECRECT_KEY;
//  config.token_status_callback = tokenStatusCallback;
//  config.max_token_generation_retry = 10;
  Firebase.begin(&config, &auth);

//  Serial.println("Đang chờ lấy User ID...");
//  while ((auth.token.uid) == "") {
//    Serial.print('.');
//    delay(1000);
//  }
  // Print user UID
  Serial.print("User ID: ");
 // Serial.println(auth.token.uid.c_str());
  Serial.println("Kết nối tới Firebase thành công!");
}
/*========= END OF CONNECTION MEDTHODS =========*/

/*========= SEND FLOAT VALUES =========*/
void sendGasData(void *para) {
  while (1) {
    MQ2.update();
    float value = MQ2.readSensor();
    gasPPM = value;
    setFloat(SENSOR_BASE_PATH + "gasPPM", value);
    delay(1000);
  }
}

void sendTemperatureData(void *para) {
  float tempValue;
  while (1) {
    tempValue = dht.readTemperature();
     Serial.print("temp");
    Serial.println(tempValue);
    if (!isnan(tempValue)) {
      setFloat(SENSOR_BASE_PATH + "temperature", tempValue);
    }
    delay(1000);
  }
}

void sendHumidityData(void *para) {
  float humidValue ;
  while (1) {
    humidValue = dht.readHumidity();
         Serial.print("humid");
    Serial.println(humidValue);
    if (!isnan(humidValue)) {
      setFloat(SENSOR_BASE_PATH + "humidity", humidValue);
    }
    delay(1000);
  }
}

void sendDustData(void *para) {
  int samplingTime = 280;
  int deltaTime = 40;
  int sleepTime = 9680;

  float voMeasured = 0;
  float calcVoltage = 0;
  float dustDensity = 0;
  while (1) {
      digitalWrite(DUST_3, LOW);
      delayMicroseconds(samplingTime);
      voMeasured = analogRead(DUST_5);
      delayMicroseconds(deltaTime);
      digitalWrite(DUST_3, HIGH);
      delayMicroseconds(sleepTime);
      calcVoltage = voMeasured * (5.0 / 4096);

      dustDensity = 172 * calcVoltage - 0.1;
      if (dustDensity < 0) dustDensity = 0;

      setFloat(SENSOR_BASE_PATH + "dustDensity", dustDensity);  
      delay(1000);
    
  

  }
}
/*========= END OF SEND FLOAT VALUES  =========*/

/*========= SEND BOOL VALUES  =========*/
void setOnFire(bool alert) { setBool(ON_BASE_PATH + "onFire", alert); }
void setOnGas(bool alert) { setBool(ON_BASE_PATH + "onGas", alert); }

void fireAlertTask(void *para) {
  int value;
  while (1) {
    value = analogRead(ANALOG_FIRE);
    if (value < FIRE_LOWER_THRESHOLD) {
      onFire = true;
      setOnFire(true);
      if (getFireAlertEnabled()) {
        digitalWrite(ANALOG_WHISTLE, LOW);
      } else {
        digitalWrite(ANALOG_WHISTLE, HIGH);
      }
    } else {
      onFire = false;
      setOnFire(false);
      digitalWrite(ANALOG_WHISTLE, HIGH);
    }
    delay(CHECKING_GAS_FIRE_TIME_2);
  }
}

void gasAlertTask(void *para) {
  setOnGas(false);
  while (1) {
    if (gasPPM > GASPPM_GREATER_THRESHOLD) {
      if (onFire == false && getGasAlertEnabled()) {
        digitalWrite(ANALOG_WHISTLE, LOW);
        delay(500);
        digitalWrite(ANALOG_WHISTLE, HIGH);
      }
      setOnGas(true);
    } else {
      setOnGas(false);
    }
    delay(CHECKING_GAS_FIRE_TIME_2);
  }
}
/*========= END OF SEND BOOL VALUES  =========*/

/*========= VALUE GETTER AND SETTER =========*/
void setFloat(String path, float value) {
  String message = path + " - ";
  if (xSemaphoreTake(semaphore, (TickType_t)10) == pdTRUE) {
    if (Firebase.setFloatAsync(fbdo, path, value)) {
//      message.concat(value);
//      Serial.println(message);
    } else {
      message.concat(" sent failed, REASON: ");
      Serial.println(message);
      Serial.println(fbdo.errorReason().c_str());
    }
    xSemaphoreGive(semaphore);
  }
}

void setBool(String path, bool value) {
  String message = path + " - ";
  if (xSemaphoreTake(semaphore, (TickType_t)10) == pdTRUE) {
    if (Firebase.setBoolAsync(fbdo, path, value)) {
//      message.concat(value);
//      Serial.println(message);
    } else {
      message.concat(" sent failed, REASON: ");
      Serial.println(message);
      Serial.println(fbdo.errorReason().c_str());
    }
    xSemaphoreGive(semaphore);
  }
}

bool getGasAlertEnabled() { return gasAlert; }
bool getFireAlertEnabled() { return fireAlert; }
/*========= END OF VALUE GETTER AND SETTER =========*/

/*========= CONFIG METHODS =========*/
void configIO() {
  pinMode(ANALOG_FIRE, INPUT);
  pinMode(ANALOG_GAS, INPUT);
  pinMode(DUST_5, INPUT);
  pinMode(ANALOG_TEHU, INPUT);

  pinMode(DUST_3, OUTPUT);
  pinMode(ANALOG_WHISTLE, OUTPUT);
  digitalWrite(ANALOG_WHISTLE, HIGH);
}
void configLed() {
  // config blinking
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 500 * 1000, true);  // delay tính ở micrso second => * 1000 để về milisecond

  // config ports
  ledcSetup(LED_RED_PWM, LED_FREQUENCY, LED_RESOLUTION);
  ledcSetup(LED_GREEN_PWM, LED_FREQUENCY, LED_RESOLUTION);
  ledcSetup(LED_BLUE_PWM, LED_FREQUENCY, LED_RESOLUTION);

  ledcAttachPin(LED_RED, LED_RED_PWM);
  ledcAttachPin(LED_GREEN, LED_GREEN_PWM);
  ledcAttachPin(LED_BLUE, LED_BLUE_PWM);
}

void configGasSensor() {
  MQ2.setRegressionMethod(1);  //_PPM =  a*ratio^b
  MQ2.setA(574.25);
  MQ2.setB(-2.222);

  /* Exponential regression:
    Gas    | a      | b
    H2     | 987.99 | -2.162
    LPG    | 574.25 | -2.222
    CO     | 36974  | -3.109
    Alcohol| 3616.1 | -2.675
    Propane| 658.71 | -2.168 */

  MQ2.init();
  MQ2.setR0(R0);
}

void configTehuSensor() { dht.begin(); }

void configTasks() {
  xTaskCreate(fireAlertTask, "Fire Alert Task", 6000, NULL, 1, NULL);
  xTaskCreate(gasAlertTask, "Gas Alert Task", 6000, NULL, 1, NULL);
  xTaskCreate(sendTemperatureData, "Sending Temperature Task", 6000, NULL, 1, NULL);
  xTaskCreate(sendHumidityData, "Sending Humidity Task", 6000, NULL, 1, NULL);
  xTaskCreate(sendGasData, "Sending Gas Task", 6000, NULL, 1, NULL);
  xTaskCreate(sendDustData, "Sending Dust Task", 6000, NULL, 1, NULL);
}

void configWatchDog() { esp_task_wdt_init(30, false); }
/*========= END OF CONFIG METHODS =========*/

void setup() {
  initSemaphore();
  Serial.begin(115200);
  connectToWifi();
  connectToFirebase();

//  loadSettings();
//  if (beginSettingChangesListener()) {
    configIO();
//    configLed();
    configGasSensor();
    configTehuSensor();
    configTasks();
//  } else {
//    Serial.println("ERROR - Lỗi khi cố gắng thiết lập kết nối nhận giá trị thay đổi, ngừng chương trình...");
//  }
}

void loop() {
//  configWatchDog();
//  delay(25000); 
}
