#include "DHT.h"
#include <FirebaseESP32.h>
#include <MQUnifiedsensor.h>
#include <WiFi.h>
#include <addons/TokenHelper.h>
#include <esp_task_wdt.h>

// DEFINE PORT
#define ANALOG_GAS 32
#define ANALOG_FIRE 33
#define ANALOG_TEHU 19 // TEMPERATURE + HUMID = TEHU
#define ANALOG_WHISTLE 12

#define DUST_1 "VCC" // LED
#define DUST_2 "GND" // LED
#define DUST_3 18    // LED
#define DUST_4 "GND"
#define DUST_5 35
#define DUST_6 "VCC"

#define LED_RED 23
#define LED_GREEN 22
#define LED_BLUE 21
#define LED_RESOLUTION 8

#define LED_RED_PWM 0
#define LED_GREEN_PWM 1
#define LED_BLUE_PWM 2


#define R0 (0.58)
//-----------------

#define SAMPLING_TIME 1000

// DEFINE GAS
#define Board ("ESP-32")
#define Type ("MQ-2")
#define Voltage_Resolution (5)
#define ADC_Bit_Resolution (12)
#define RatioMQ2CleanAir (9.83) // RS / R0 = 9.83 ppm
MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, ANALOG_GAS, Type);

// DEFINE FIREBASE
#define DATABASE_URL "iot-project-7486d-default-rtdb.asia-southeast1.firebasedatabase.app"
#define SECRECT_KEY "t9gujxIcFO6k3k6V2LX71UO89UMReJ8rVghlMuW5"
#define API_KEY "AIzaSyDExcDgLNEP2xf2hxtQS-nZLdFBtK-2ZzM"

#define USER_EMAIL "minhtu1392000@gmail.com"
#define USER_PASSWORD "123456789"
// DEFINE WIFI
#define WIFI_SSID "Asura"
#define WIFI_PASSWORD "30121995@sura"

// DEFINE DHT
#define DHTTYPE DHT22
DHT dht(ANALOG_TEHU, DHTTYPE);

// VARIABLES
float gasPPM = 0;
bool onFire = false;

FirebaseData fbdo;
FirebaseData stream;
FirebaseAuth auth;
FirebaseConfig config;

//Dùng define ko nối chuỗi dc
const String SENSOR_BASE_PATH = "values/";
const String ON_BASE_PATH = "values/";
const String ALERT_BASE_PATH = "settings/alert/";
const String LED_BASE_PATH = "settings/led/";

bool gasAlert;
bool fireAlert;

//LED VARIABLES
int ledBrightness = 50, ledRed = 255, ledGreen = 255, ledBlue = 255
bool ledIsOn = false, ledIsBlinking = false;
String ledRGBHex = "FFFFFF";


SemaphoreHandle_t semaphore = NULL;

void timeOutCallBack(bool timeout) {
  if (timeout) {
    Serial.println("Quá thời hạn callback(timeout) \n");
  }
  if (!stream.httpConnected()) {
    Serial.printf("Lỗi callback: %d, lý do: %s\n\n", stream.httpCode(),
                  stream.errorReason().c_str());
  }
}

void alertChangedCallBack(StreamData data) {
  if (data.dataTypeEnum() == fb_esp_rtdb_data_type_boolean) {
    if (strcmp(data.dataPath().c_str(),"/gas") == 0) {
      gasAlert = data.to<bool>();
    } else 
    if (strcmp(data.dataPath().c_str(),"/fire") == 0) {
      fireAlert = data.to<bool>();
    }
  } else {
    Serial.println("Sai định dạng, ko update bool alert");
  }
}

void ledChangedCallBack(StreamData data){
  if(strcmp(data.dataPath().c_str(),"/brightness") == 0){
    
  }
  if(strcmp(data.dataPath().c_str(),"/RGB") == 0){
    
  }
  if(strcmp(data.dataPath().c_str(),"/isBlinking") == 0){
    
  }
  if(strcmp(data.dataPath().c_str(),"/isOn") == 0){
    
  }
}



void ledChangedCallBack(StreamData data) {
  if (data.dataTypeEnum() == fb_esp_rtdb_data_type_boolean) {
    if (strcmp(data.dataPath().c_str(),"/gas") == 0) {
      gasAlert = data.to<bool>();
    } else 
    if (strcmp(data.dataPath().c_str(),"/fire") == 0) {
      fireAlert = data.to<bool>();
    }
  } else {
    Serial.println("Sai định dạng, ko update bool alert");
  }
}



void initSemaphore() {
  semaphore = xSemaphoreCreateBinary();
  if (semaphore != NULL) {
    xSemaphoreGive(semaphore);
  }
}

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
 
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  config.token_status_callback = tokenStatusCallback;
  config.max_token_generation_retry = 5;
  Firebase.begin(&config, &auth);

  Serial.println("Đang lấy User ID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  Serial.print("User ID: ");
  Serial.println(auth.token.uid.c_str());
  Serial.println("Kết nối tới Firebase thành công!");
}

// SEND DATA
void sendGasData(void *para) {
  while (1) {
    MQ2.update();
    float value = MQ2.readSensor();
    gasPPM = value;
    setFloat(SENSOR_BASE_PATH + "gasPPM", value);
    delay(SAMPLING_TIME);
  }
}

void sendTemperatureData(void *para) {
  while (1) {
    float tempValue = dht.readTemperature();
    if (!isnan(tempValue)) {
      setFloat(SENSOR_BASE_PATH + "temperature", tempValue);
    }
    delay(SAMPLING_TIME);
  }
}

void sendHumidityData(void *para) {
  while (1) {
    float humidValue = dht.readHumidity();
    if (!isnan(humidValue)) {
      setFloat(SENSOR_BASE_PATH + "humidity", humidValue);
    }
    delay(SAMPLING_TIME);
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
    if (dustDensity < 0)
      dustDensity = 0;

    setFloat(SENSOR_BASE_PATH + "dustDensity", dustDensity);
    delay(SAMPLING_TIME);
  }
}
// SETTER
void setFloat(String path, float value) {
  String message = path + " - ";
  if (xSemaphoreTake(semaphore, (TickType_t)10) == pdTRUE) {
    if (Firebase.setFloatAsync(fbdo, path, value)) {
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
    } else {
      message.concat(" sent failed, REASON: ");
      Serial.println(message);
      Serial.println(fbdo.errorReason().c_str());
    }
    xSemaphoreGive(semaphore);
  }
}
//--------ALERT--------
bool getGasAlertEnabled() {
//  if (xSemaphoreTake(semaphore, (TickType_t)10) == pdTRUE) {
//    bool result;
//    if(!Firebase.getBool(fbdo, ALERT_BASE_PATH + "gas", &result)){//Trường hợp ko get dc, thì coi như là false
//      xSemaphoreGive(semaphore);
//      return false;
//    }
//    Serial.println("Gas allowed: ");
//    Serial.println(result);
//    xSemaphoreGive(semaphore);
//    return result; 
//  }
return gasAlert;
}

bool getFireAlertEnabled() {
return fireAlert
}

void setOnFire(bool alert) { 
  setBool(ON_BASE_PATH + "onFire", alert); 
}

void setOnGas(bool alert) { 
  setBool(ON_BASE_PATH + "onGas", alert); 
}

void fireAlertTask(void *para) {
  int value;
  // delay(200);
  while (1) {
    value = analogRead(ANALOG_FIRE);
    if (value < 3300) {
      onFire = true;
      setOnFire(true);
      if (getFireAlertEnabled()) {
        digitalWrite(ANALOG_WHISTLE, LOW);
      }else{
        digitalWrite(ANALOG_WHISTLE, HIGH);
      }
    } else {
      onFire = false;
      setOnFire(false);
      digitalWrite(ANALOG_WHISTLE, HIGH);
    }
    delay(250);
  }
}

void gasAlertTask(void *para) {
  setOnGas(false);
  while (1) {
    if (gasPPM > 1000) {
      if (onFire == false && getGasAlertEnabled()) {
        digitalWrite(ANALOG_WHISTLE, LOW);
        delay(500);
        digitalWrite(ANALOG_WHISTLE, HIGH);
      }
      setOnGas(true);
    } else {
      setOnGas(false);
    }
    delay(250);
  }
}

// CONFIG
void configIO() {
  pinMode(ANALOG_FIRE, INPUT);
  pinMode(ANALOG_GAS, INPUT);
  pinMode(DUST_5, INPUT);
  pinMode(ANALOG_TEHU, INPUT);

  pinMode(DUST_3, OUTPUT);
  pinMode(ANALOG_WHISTLE, OUTPUT);
  digitalWrite(ANALOG_WHISTLE, HIGH);
}

void configGasSensor() {
  MQ2.setRegressionMethod(1); //_PPM =  a*ratio^b
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

void feedWatchDog() { esp_task_wdt_init(30, false); }

void setup() {
  initSemaphore();
  Serial.begin(9600);
  connectToWifi();
  connectToFirebase();

  configIO();
  configGasSensor();
  configTehuSensor();
  configTasks();
}

void loop() {
  feedWatchDog();
  delay(25000);
}
