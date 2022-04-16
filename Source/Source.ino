#include <FirebaseESP32.h>
#include <MQUnifiedsensor.h>
#include <esp_task_wdt.h>
#include "DHT.h"
#include <WiFi.h>
#include <addons/RTDBHelper.h>

// DEFINE PORT
#define ANALOG_GAS 33
#define ANALOG_FIRE 32
#define ANALOG_TEHU 18 //TEMPERATURE + HUMID = TEHU
#define ANALOG_WHISTLE 13

#define DUST_1 "VCC" //LED
#define DUST_2 "GND" //LED
#define DUST_3 4 //LED
#define DUST_4 "GND"
#define DUST_5 34
#define DUST_6 "VCC"

#define LED_RED 23
#define LED_GREEN 22
#define LED_BLUE 1

//-----------------

#define SAMPLING_TIME 1000
// DEFINE GAS
#define Board              ("ESP-32")
#define Type               ("MQ-2")
#define Voltage_Resolution (5)
#define ADC_Bit_Resolution (12)
#define RatioMQ2CleanAir   (9.83) //RS / R0 = 9.83 ppm
MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, ANALOG_GAS, Type);

// DEFINE FIREBASE
#define DATABASE_URL "esp32-server-4434f-default-rtdb.asia-southeast1.firebasedatabase.app"
#define SECRECT_KEY "Ru1XxrYtlxMNZyvv4jR4CTKVFDMpBN0uzIa5gJID"

// DEFINE WIFI
#define WIFI_SSID "Asura"
#define WIFI_PASSWORD "30121995@sura"

// DEFINE DHT
#define DHTTYPE DHT22

DHT dht(ANALOG_TEHU, DHTTYPE);

//VARIABLES
float gasPPM = 0;
bool onFire = false;
float R0 = 0.58;
float newR0;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String SENSOR_BASE_PATH = "sensor/";
String ALERT_BASE_PATH  = "alert/";

SemaphoreHandle_t semaphore = NULL;

//DUST PRIVATE VARIABLES
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;

float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;
//----------------

void initSemaphore(){
  semaphore = xSemaphoreCreateBinary();
  if(semaphore != NULL){
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
    config.database_url = DATABASE_URL;
    config.signer.tokens.legacy_token = SECRECT_KEY;
    Firebase.reconnectWiFi(true);
    Firebase.begin(&config, &auth);
}


// SEND DATA
void sendGasData(void * para) {
  while (1) {
    MQ2.update();
    float value = MQ2.readSensor();
    gasPPM = value;
    // printGas(value);
    setFloat(SENSOR_BASE_PATH + "gasPPM", value);
    delay(SAMPLING_TIME);
  }
}

void sendTemperatureData(void * para) {
  while (1) {
    float tempValue = dht.readTemperature();
    // printTemperature(tempValue);
    if(!isnan(tempValue)){
      setFloat(SENSOR_BASE_PATH + "temperature", tempValue);
    }
    delay(SAMPLING_TIME);
  }
}

void sendHumidityData(void * para) {
  while (1) {
    float humidValue = dht.readHumidity();
    // printHumidity(humidValue);
    if(!isnan(humidValue)){
      setFloat(SENSOR_BASE_PATH + "humidity", humidValue);
    }
    delay(SAMPLING_TIME);
  }
}

void sendDustData(void * para) {
  while (1) {
    /*--------PROCESSING--------*/

    digitalWrite(DUST_3, LOW); // Bật IR LED
    delayMicroseconds(samplingTime); //Delay 0.28ms
    voMeasured = analogRead(DUST_5); // Đọc giá trị ADC V0
    delayMicroseconds(deltaTime); //Delay 0.04ms
    digitalWrite(DUST_3, HIGH); // Tắt LED
    delayMicroseconds(sleepTime); //Delay 9.68ms

    // Tính điện áp từ giá trị ADC
    calcVoltage = voMeasured * (5.0 / 4095); //Điệp áp Vcc của cảm biến (5.0 hoặc 3.3)

    // Linear Equation http://www.howmuchsnow.com/arduino/airquality/
    // Chris Nafis (c) 2012
    dustDensity = 172 * calcVoltage - 0.0999;
    if (dustDensity < 0) dustDensity = 0;

    /*--------END PROCESSING--------*/

    // printDustDensity(dustDensity);

    setFloat(SENSOR_BASE_PATH + "dustDensity", dustDensity);
    delay(SAMPLING_TIME);
  }
}

String getAQILevel(float dustDensity) {
  if (dustDensity <= 12) return "Tốt";
  if (dustDensity <= 35.4) return "Trung bình";
  if (dustDensity <= 150.4) return "Kém";
  if (dustDensity <= 250.4) return "Xấu";
  if (dustDensity <= 350.4) return "Rất xấu";
  return "Nguy hại";
}

//--------ALERT--------
void printFireAlert() {
  Serial.println("======");
  Serial.println(">> CẢNH BÁO CÓ LỬA <<");
  Serial.println("======");
}
void printGasAlert() {
  Serial.println("======");
  Serial.println(">> CẢNH BÁO NỒNG ĐỘ GA CAO <<");
  Serial.println("======");
}
void alertFire(bool alert) {
  setBool(ALERT_BASE_PATH + "fire", alert);
}

void alertGas(bool alert) {
  setBool(ALERT_BASE_PATH + "gas", alert);
}

void fireAlertTask(void * para) {
  bool previousOnFire = false;
  alertFire(false);
  delay(200);
  while (1) {
    if (analogRead(ANALOG_FIRE) < 3300) {  
      onFire = true;
      if(previousOnFire == false){
        previousOnFire = true;
        alertFire(true);
        digitalWrite(ANALOG_WHISTLE, LOW);
      }
    } else {
      onFire = false;
      if(previousOnFire == true){
        previousOnFire = false;
        alertFire(false);
        digitalWrite(ANALOG_WHISTLE, HIGH);
      }
    }
    Serial.println("FIRE");
    Serial.println(previousOnFire);
    delay(200);
  }
}

void gasAlertTask(void * para) {
  bool previousOnGas = false;
  alertGas(false);
  delay(200);
  while (1) {
    if (gasPPM > 3000) {
      if(onFire == false){
        digitalWrite(ANALOG_WHISTLE, LOW);
        delay(500);
        digitalWrite(ANALOG_WHISTLE, HIGH);
        delay(300);
      }
      if(previousOnGas == false){
        previousOnGas = true;
        alertGas(true);
        Serial.println("ALERTED GAS TRUE");
      }
    } else {
      if(previousOnGas == true){
        previousOnGas = false;
        alertGas(false);
        Serial.println("ALERTED GAS FALSE");
      }
    }
    Serial.println("GAS");
    Serial.println(previousOnGas);
    delay(200);
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
  // newR0 = CalibrateR0();
  // MQ2.setR0(newR0);
}

float CalibrateR0() {
  Serial.println("Calibrating R0");
  float calcR0 = 0;
  for (int i = 1; i <= 500; i++) {
    MQ2.update();
    calcR0 += MQ2.calibrate(RatioMQ2CleanAir);
    Serial.print(".");
  }
  calcR0 = calcR0 / 500;
  Serial.print("Calibrated R0: ");
  Serial.println(calcR0);
  return calcR0;
}

void configTehuSensor() {
  dht.begin();
}

void configTasks() {
  xTaskCreate(fireAlertTask, "Fire Alert Task", 6000, NULL, 1, NULL);
  xTaskCreate(gasAlertTask, "Gas Alert Task", 6000, NULL, 1, NULL);
  xTaskCreate(sendDustData, "Sending Dust Task", 6000, NULL, 1, NULL);
  xTaskCreate(sendTemperatureData, "Sending Temperature Task", 6000, NULL, 1, NULL);
  xTaskCreate(sendHumidityData, "Sending Humidity Task", 6000, NULL, 1, NULL);
  xTaskCreate(sendGasData, "Sending Gas Task", 6000, NULL, 1, NULL);
}

void feedWatchDog() {
//  TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE; //enable register - cho phép feed
//  TIMERG0.wdt_feed = 1;
//  TIMERG0.wdt_wprotect = 0; //đóng feed
esp_task_wdt_init(30, false);
}

//SETTER
void setFloat(String path, float value) {
  String message = path + " with value ";
  if( xSemaphoreTake( semaphore, ( TickType_t ) 10 ) == pdTRUE ){
    if(Firebase.setFloat(fbdo, path, value)){
      message.concat(value);
      message.concat(" sent successfully!");
      Serial.println(message);
    }else{
      message.concat(" sent failed, REASON: ");
      Serial.println(message);
      Serial.println(fbdo.errorReason().c_str());
    }  
    xSemaphoreGive( semaphore );
  }

}

void setBool(String path, bool value) {
  String message = path + " with value ";
  if( xSemaphoreTake( semaphore, ( TickType_t ) 10 ) == pdTRUE ){
    if(Firebase.setBool(fbdo, path, value)){
      message.concat(value);
      message.concat(" sent successfully!");
      Serial.println(message);
    }else{
      message.concat(" sent failed, REASON: ");
      Serial.println(message);
      Serial.println(fbdo.errorReason().c_str());
    }    
    xSemaphoreGive( semaphore );
  }

}

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
