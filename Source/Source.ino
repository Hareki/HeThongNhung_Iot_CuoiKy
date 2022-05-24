#include <DHT.h>
#include <FirebaseESP32.h>
#include <GP2YDustSensor.h>
#include <MQUnifiedsensor.h>
#include <WiFi.h>
#include <addons/TokenHelper.h>
#include <esp_task_wdt.h>

/*========= DEFINE PORTS =========*/
#define ANALOG_GAS 36
#define ANALOG_FIRE 33
#define DIGITAL_DHT22 19  // TEMPERATURE + HUMID = TEHU
#define ANALOG_WHISTLE 12

#define DUST_1 "VCC"  // LED
#define DUST_2 "GND"  // LED
#define DUST_3 4      // LED
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
#define R0 (8.8)
#define Board ("ESP-32")
#define Type ("MQ-2")
#define Voltage_Resolution (3.3)
#define ADC_Bit_Resolution (12)
#define RatioMQ2CleanAir (9.83)  // RS / R0 = 9.83 ppm
MQUnifiedsensor gasSensor(Board, Voltage_Resolution, ADC_Bit_Resolution, ANALOG_GAS, Type);

GP2YDustSensor dustSensor(GP2YDustSensorType::GP2Y1010AU0F, DUST_3, DUST_5);

#define DHTTYPE DHT22
DHT dht(DIGITAL_DHT22, DHTTYPE);

/*========= END OF DEFINE SENSORS =========*/

/*========= DEFINE COMMON VARIABLES =========*/
#define GASPPM_GREATER_THRESHOLD 800
#define FIRE_LOWER_THRESHOLD 3300
#define SAMPLING_TIME 1000

#define LED_FREQUENCY 5000
#define LED_RESOLUTION 8

float gasPPM = 0;     // biến giao tiếp giữa onGasCheckingTask() và sendGasData()
bool onFire = false;  // biến giao tiếp giữa onGasCheckingTask() và onFireCheckingTask()

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

FirebaseData stream;
bool dataIsNode(String node) { return strstr(stream.dataPath().c_str(), node.c_str()); }

bool dataIsInt() { return stream.dataTypeEnum() == fb_esp_rtdb_data_type_integer; }
bool dataIsBool() { return stream.dataTypeEnum() == fb_esp_rtdb_data_type_boolean; }
bool dataIsString() { return stream.dataTypeEnum() == fb_esp_rtdb_data_type_string; }
bool dataIsNull() { return stream.dataTypeEnum() == fb_esp_rtdb_data_type_null; }
bool dataIsJson() { return stream.dataTypeEnum() == fb_esp_rtdb_data_type_json; }

void loadSettings() {
  Serial.println("Đang lấy giá trị độ sáng led");
  while (!Firebase.getInt(fbdo, "settings/led/brightness")) {
    Serial.print(".");
  }
  ledBrightness = fbdo.to<int>();
  setBrightness(ledBrightness);
  Serial.println(ledBrightness);

  Serial.println("Đang lấy giá trị nhấp nháy led");
  while (!Firebase.getBool(fbdo, "settings/led/isBlinking")) {
    Serial.print(".");
  }
  ledIsBlinking = fbdo.to<bool>();
  setBlinking(ledIsBlinking);
  Serial.println(ledIsBlinking);

  Serial.println("Đang lấy giá trị nguồn led");
  while (!Firebase.getBool(fbdo, "settings/led/isOn")) {
    Serial.print(".");
  }
  ledIsOn = fbdo.to<bool>();
  setPower(ledIsOn);
  Serial.println(ledIsOn);

  Serial.println("Đang lấy giá trị RGB led");
  while (!Firebase.getString(fbdo, "settings/led/RGB")) {
    Serial.print(".");
  }
  ledRGBHex = fbdo.to<String>();
  setRGB(ledRGBHex);
  Serial.println(ledRGBHex);

  Serial.println("Đang lấy giá trị cảnh báo gas");
  while (!Firebase.getBool(fbdo, "settings/alert/gas")) {
    Serial.print(".");
  }
  gasAlert = fbdo.to<bool>();
  Serial.println(gasAlert);

  Serial.println("Đang lấy giá trị cảnh báo lửa");
  while (!Firebase.getBool(fbdo, "settings/alert/fire")) {
    Serial.print(".");
  }
  fireAlert = fbdo.to<bool>();
  Serial.println(fireAlert);
}


/*=========END OF SETTINGS UPDATER REGION=========*/

/*========= LED CONTROL MEDTHODS =========*/
void setBrightness(int brightness) {
  if (ledIsOn || brightness == 0) { //led tắt => set brightness = 0
    double percent = (double)brightness / (double)100;
    int realLedRedValue = round((double)ledRedValue * percent);
    int realLedGreenValue = round((double)ledGreenValue * percent);
    int realLedBlueValue = round((double)ledBlueValue * percent);

    ledcWrite(LED_RED_PWM, realLedRedValue);
    ledcWrite(LED_GREEN_PWM, realLedGreenValue);
    ledcWrite(LED_BLUE_PWM, realLedBlueValue);
  }
}

void setPower(bool isOn) {
  if (isOn) {
    setBrightness(ledBrightness);
  } else {
    setBrightness(0);
  }
}

/*--blinking--*/
bool ledState = true; //=> Biến điều khiển nhấp nháy
bool blinkingEnable = false;
void blinkingTask(void *para) {
  const TickType_t delayTime = 500 / portTICK_PERIOD_MS;
  while (1) {
//    Serial.printf("Enable: %d", blinkingEnable);
//    Serial.printf("Power: %d", ledIsOn);
    if (blinkingEnable && ledIsOn) {
      if (ledState) {
        ledState = false;
        setBrightness(0);
      } else {
        ledState = true;
        setBrightness(ledBrightness);
      }
    }
    vTaskDelay( delayTime );
    //    Serial.printf("Blinking stack size: %d\n\n", uxTaskGetStackHighWaterMark(NULL));
  }
}

void setBlinking(bool isBlinking) {
  blinkingEnable = isBlinking;
  setBrightness(ledBrightness);
}
/*--blinking--*/

void convertHexToRGB(String hexString) {
  int hexValue = (int)strtol(hexString.c_str(), NULL, 16);
  ledRedValue = ((hexValue >> 16) & 0xFF);
  ledGreenValue = ((hexValue >> 8) & 0xFF);
  ledBlueValue = ((hexValue)&0xFF);
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
  config.database_url = DATABASE_URL;
  Firebase.reconnectWiFi(true);
  config.signer.tokens.legacy_token = SECRECT_KEY;
  Firebase.begin(&config, &auth);
  Serial.println("Kết nối tới Firebase thành công!");
}
/*========= END OF CONNECTION MEDTHODS =========*/

/*========= SEND FLOAT VALUES =========*/

void sendGasData() {
  gasSensor.update();
  float value = gasSensor.readSensor();
  gasPPM = value;
  setFloat(SENSOR_BASE_PATH + "gasPPM", value);
}

float tempValue;
void sendTemperatureData() {
  tempValue = dht.readTemperature();
  if (!isnan(tempValue)) {
    setFloat(SENSOR_BASE_PATH + "temperature", tempValue);
  } else {
    Serial.println("TEMP FAILED");
  }
}

float humidValue;
void sendHumidityData() {
  humidValue = dht.readHumidity();
  if (!isnan(humidValue)) {
    setFloat(SENSOR_BASE_PATH + "humidity", humidValue);
  } else {
    Serial.println("HUMID FAILED");
  }
}

void sendDustData() {
  float dustDensity = dustSensor.getDustDensity();
  setFloat(SENSOR_BASE_PATH + "dustDensity", dustDensity);
}
/*========= END OF SEND FLOAT VALUES  =========*/

/*========= SEND BOOL VALUES  =========*/
void setOnFire(bool alert) { setBool(ON_BASE_PATH + "onFire", alert); }
void setOnGas(bool alert) { setBool(ON_BASE_PATH + "onGas", alert); }

void onFireCheckingTask(void *para) {
  int value;
  const TickType_t delayTime = 250 / portTICK_PERIOD_MS;
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
    //   Serial.printf("Fire stack size: %d\n\n", uxTaskGetStackHighWaterMark(NULL));
    vTaskDelay(delayTime);
  }
}

void onGasCheckingTask(void *para) {
  TickType_t lastWakeTime;
  lastWakeTime = xTaskGetTickCount ();
  
  const TickType_t delayTime = 500;
  while (1) {
      if (gasPPM > GASPPM_GREATER_THRESHOLD) {
      if (onFire == false && getGasAlertEnabled()) {
        digitalWrite(ANALOG_WHISTLE, LOW);
        xTaskDelayUntil( &lastWakeTime, delayTime );
        digitalWrite(ANALOG_WHISTLE, HIGH);
      }
      setOnGas(true);
    } else {
      setOnGas(false);
    }
    xTaskDelayUntil( &lastWakeTime, delayTime );
  }
}
/*========= END OF SEND BOOL VALUES  =========*/

/*========= VALUE GETTER AND SETTER =========*/
void setFloat(String path, float value) {
  String message = path + " - ";
  if (xSemaphoreTake(semaphore, (TickType_t)10 / portTICK_PERIOD_MS) == pdTRUE) {
    if (Firebase.setFloatAsync(fbdo, path, value)) { //Async = realtime
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
  if (xSemaphoreTake(semaphore, (TickType_t)10 / portTICK_PERIOD_MS) == pdTRUE) {
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
  //  pinMode(DIGITAL_DHT22, INPUT); - không cần, trong thư viện tự set

  pinMode(DUST_3, OUTPUT);
  pinMode(ANALOG_WHISTLE, OUTPUT);
  digitalWrite(ANALOG_WHISTLE, HIGH);
}
void configLed() {
  // config ports
  ledcSetup(LED_RED_PWM, LED_FREQUENCY, LED_RESOLUTION);
  ledcSetup(LED_GREEN_PWM, LED_FREQUENCY, LED_RESOLUTION);
  ledcSetup(LED_BLUE_PWM, LED_FREQUENCY, LED_RESOLUTION);

  ledcAttachPin(LED_RED, LED_RED_PWM);
  ledcAttachPin(LED_GREEN, LED_GREEN_PWM);
  ledcAttachPin(LED_BLUE, LED_BLUE_PWM);
}

void configDustSensor() {
  dustSensor.setSensitivity(0.5);
  dustSensor.setBaseline(0.2);
}

void configGasSensor() {
  gasSensor.setRegressionMethod(1);  //_PPM =  a*ratio^b
  gasSensor.setA(574.25);
  gasSensor.setB(-2.222);

  /* Exponential regression:
    Gas    | a      | b
    H2     | 987.99 | -2.162
    LPG    | 574.25 | -2.222
    CO     | 36974  | -3.109
    Alcohol| 3616.1 | -2.675
    Propane| 658.71 | -2.168 */

  gasSensor.init();
  gasSensor.setR0(R0);
}

void configTehuSensor() { dht.begin(); }

void configTasks() {
  xTaskCreatePinnedToCore(onFireCheckingTask, "FireTask", 10000, NULL, 0, NULL, 0);
  xTaskCreatePinnedToCore(onGasCheckingTask, "GasTask", 10000, NULL, 0, NULL, 0);
  xTaskCreatePinnedToCore(blinkingTask, "BlinkingTask", 10000, NULL, 0, NULL, 0);
}

void configWatchDog() { esp_task_wdt_init(30, false); }
/*========= END OF CONFIG METHODS =========*/

bool enable = true;
void setup() {
  initSemaphore();
  Serial.begin(9600);
  connectToWifi();
  connectToFirebase();

  configIO();
  configGasSensor();
  configDustSensor();
  configTehuSensor();
  configLed();

  loadSettings();  // Load setting và config đèn, biến theo setting đó
  configTasks();   // Phải để sau loadSettings, vì configTask xong là nó chạy ngay
  Serial.println("Đang set stream...");
  if (!Firebase.beginStream(stream, SETTING_BASE_PATH)) {
    Serial.printf("ERROR - Xảy ra lỗi khi set stream: , %s\n\n", stream.errorReason().c_str());
    Serial.println("Tiến hành ngắt chương trình...");
    enable = false;
  }
}

unsigned long interval1 = 0;
unsigned long interval2 = 0;
unsigned long interval3 = 0;
void loop() {
  if (enable) { //Dùng loop vì temp humid, dust fail quá nhiều!, liên quan về tính chính xác về time của sensor
    if ((unsigned long)(millis() - interval1) > 1000 || interval1 == 0) {
      interval1 = millis();
//      Serial.println("==================");
      sendDustData();
      sendTemperatureData();
      sendHumidityData();
      sendGasData();
    }

    if ((unsigned long)(millis() - interval2) > 2000 || interval2 == 0) { // Cảm biến chậm, lấy mẫu nhanh hơn đọc ra kq như cũ => tốn tài nguyên set lên db kq cũ
      interval2 = millis();
      sendTemperatureData();
      sendHumidityData();
    }
    if ((unsigned long)(millis() - interval3) > 25000 || interval3 == 0) {
      interval3 = millis();
      configWatchDog();
    }

    if (!Firebase.readStream(stream))
      Serial.printf("ERROR - Không thể đọc dữ liệu từ stream, %s\n\n", stream.errorReason().c_str());

    if (stream.streamAvailable()) {
      if (!dataIsJson()) {
        if (dataIsBool()) {//Kiểm tra cho chắc, tránh việc ép kiểu sai, crash chương trình
          if (dataIsNode("isBlinking")) {
            ledIsBlinking = stream.to<bool>();
            Serial.printf("VALUE CHANGED - BLINKING: %d\n", ledIsBlinking);
            setBlinking(ledIsBlinking);
          } else if (dataIsNode("isOn")) {
            ledIsOn = stream.to<bool>();
            Serial.printf("VALUE CHANGED - ON: %d\n", ledIsOn);
            setPower(ledIsOn);
          } else if (dataIsNode("gas")) {
            gasAlert = stream.to<bool>();
            Serial.printf("ALERT GAS - ON: %d\n", gasAlert);
          } else if (dataIsNode("fire")) {
            fireAlert = stream.to<bool>();
            Serial.printf("ALERT FIRE - ON: %d\n", fireAlert);
          }
        } else if (dataIsString() && dataIsNode("RGB")) {
          ledRGBHex = stream.to<String>();
          Serial.printf("HEX RGB: %s", ledRGBHex);
          setRGB(ledRGBHex);
        } else if (dataIsInt() && dataIsNode("brightness")) {
          ledBrightness = stream.to<int>();
          Serial.printf("LED BRIGHTNESS: %d\n", ledRGBHex);
          setBrightness(ledBrightness);
        } else {
          Serial.println("ERROR - Định dạng dữ liệu thay đổi sai, không thể cập nhật");
        }
      }
    }
  }
}
