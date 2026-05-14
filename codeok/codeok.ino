#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "time.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include "driver/ledc.h"

// ================== FIREBASE ==================
const char* FIREBASE_HOST =
"https://appgiamsat-dd86a-default-rtdb.asia-southeast1.firebasedatabase.app/";

#define API_KEY "AIzaSyBc-Gb9Gzo-QSEDNUS-o1eiQ0Q2AAV3FJc"

// ================== PIN ==================
#define CAMBIEN_SANG 34
#define CAMBIEN_DHT 4

#define NUT_MODE 26
#define NUT_QUAT 32
#define NUT_DEN 35
#define NUT_SERVO 33

#define QUAT 18
#define DEN 19
#define SERVO_PIN 5

#define DHTTYPE DHT11

// ================== INIT ==================
DHT dht(CAMBIEN_DHT, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================== BIẾN ==================
volatile bool coNgatMode = false;
volatile bool coNgatQuat = false;
volatile bool coNgatDen = false;
volatile bool coNgatServo = false;

bool trangThaiQuat = false;
bool trangThaiDen = false;
bool trangThaiServo = false;

// ================== SERVO ==================
int duty_0, duty_180;

// ================== TIME ==================
struct tm timeinfo;

int giochay = -1;

bool ServoChay = false;

unsigned long tgianbatS = 0;

// ================== DEBOUNCE ==================
const int debounce = 200;

unsigned long lastMode = 0;
unsigned long lastQuat = 0;
unsigned long lastDen = 0;
unsigned long lastServoBtn = 0;

// ================== LCD ==================
bool MH = 0;

unsigned long lastLCD = 0;

// ================== FIREBASE ==================
String cheDo = "auto";

int fb_quat = 0;
int fb_den = 0;
int fb_servo = 0;

int lastServoState = -1;

// ================== AUTO SETTING ==================
int nguongNhiet = 30;

int nguongLux = 2500;

int gioBatDen = 18;

int gioChoAn = 6;

// ================== TRẠNG THÁI ==================
bool TTQuat = false;
bool TTDen = false;
bool TTServo = false;

// ================== TIMER ==================
unsigned long lastFirebase = 0;

unsigned long lastSend = 0;

// ================== WIFI ==================
void khoiTaoWiFi() {

  Serial.println("===== WIFI START =====");

  WiFiManager wm;

  if (!wm.autoConnect("ESP32-SETUP")) {

    Serial.println(" WIFI FAIL -> RESET");

    ESP.restart();
  }

  Serial.println(" WIFI CONNECTED");

  configTime(7 * 3600, 0, "pool.ntp.org");

  Serial.print(" Đang lấy thời gian...");

  while (!getLocalTime(&timeinfo)) {

    Serial.print(".");

    delay(500);
  }

  Serial.println("\n TIME OK");
}

// ================== FIREBASE READ ==================
void docFirebase() {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println(" WIFI DISCONNECTED");

    return;
  }

  HTTPClient http;

  Serial.println("===== READ FIREBASE =====");

  // ===== CONTROL =====
  String url =
    String(FIREBASE_HOST) +
    "/control.json?auth=" +
    API_KEY;

  http.begin(url);

  int code = http.GET();

  Serial.println("GET CONTROL CODE: " + String(code));

  if (code > 0) {

    String payload = http.getString();

    Serial.println("CONTROL JSON: " + payload);

    DynamicJsonDocument doc(256);

    deserializeJson(doc, payload);

    cheDo = doc["mode"] | "auto";

    fb_quat = doc["fan"] | 0;

    fb_den = doc["led"] | 0;

    fb_servo = doc["servo"] | 0;

    Serial.printf("Mode:%s | Fan:%d | Led:%d | Servo:%d\n",
                  cheDo.c_str(),
                  fb_quat,
                  fb_den,
                  fb_servo);
  }

  http.end();

  // ===== AUTO =====
  url =
    String(FIREBASE_HOST) +
    "/auto.json?auth=" +
    API_KEY;

  http.begin(url);

  code = http.GET();

  Serial.println("GET AUTO CODE: " + String(code));

  if (code > 0) {

    String payload = http.getString();

    Serial.println("AUTO JSON: " + payload);

    DynamicJsonDocument doc(256);

    deserializeJson(doc, payload);

    nguongNhiet =
      doc["temp_threshold"] | nguongNhiet;

    nguongLux =
      doc["lux_threshold"] | nguongLux;

    gioChoAn =
      doc["feed_hour"] | gioChoAn;

    Serial.printf("AutoSetting -> T:%d Lux:%d Feed:%d\n",
                  nguongNhiet,
                  nguongLux,
                  gioChoAn);
  }

  http.end();
}

// ================== FIREBASE SEND ==================
void guiFirebase(float t, float h, int lux) {

  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;

  String url =
    String(FIREBASE_HOST) +
    "/sensor.json?auth=" +
    API_KEY;

  http.begin(url);

  http.addHeader("Content-Type", "application/json");

  String json = "{";

  json += "\"temp\":" + String(t) + ",";

  json += "\"hum\":" + String(h) + ",";

  json += "\"lux\":" + String(lux);

  json += "}";

  int code = http.PUT(json);

  Serial.println("SEND SENSOR CODE: " + String(code));

  http.end();
}

// ================== SERVO ==================
void servo_0() {

  Serial.println("SERVO -> 0");

  ledc_set_duty(LEDC_HIGH_SPEED_MODE,
                LEDC_CHANNEL_0,
                duty_0);

  ledc_update_duty(LEDC_HIGH_SPEED_MODE,
                   LEDC_CHANNEL_0);
}

void servo_180() {

  Serial.println("SERVO -> 180");

  ledc_set_duty(LEDC_HIGH_SPEED_MODE,
                LEDC_CHANNEL_0,
                duty_180);

  ledc_update_duty(LEDC_HIGH_SPEED_MODE,
                   LEDC_CHANNEL_0);
}

// ================== ISR ==================
void IRAM_ATTR modeISR() {

  if (millis() - lastMode > debounce) {

    coNgatMode = true;

    lastMode = millis();
  }
}

void IRAM_ATTR quatISR() {

  if (millis() - lastQuat > debounce) {

    coNgatQuat = true;

    lastQuat = millis();
  }
}

void IRAM_ATTR denISR() {

  if (millis() - lastDen > debounce) {

    coNgatDen = true;

    lastDen = millis();
  }
}

void IRAM_ATTR servoISR() {

  if (millis() - lastServoBtn > debounce) {

    coNgatServo = true;

    lastServoBtn = millis();
  }
}

// ================== LCD ==================
void lcdSensor(float t, float h, int lux) {

  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("T:");
  lcd.print(t);

  lcd.print("C H:");
  lcd.print(h);

  lcd.print("%");

  lcd.setCursor(0, 1);

  lcd.print("Lux:");
  lcd.print(lux);
}

void lcdStatus() {

  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("Mode:");
  lcd.print(cheDo);

  lcd.setCursor(0, 1);

  lcd.print("F:");
  lcd.print(TTQuat ? "ON" : "OFF");

  lcd.print(" L:");
  lcd.print(TTDen ? "ON" : "OFF");

  lcd.print(" S:");
  lcd.print(TTServo ? "ON" : "OFF");
}

// ================== SETUP ==================
void setup() {

  Serial.begin(115200);

  Serial.println("===== SYSTEM START =====");

  khoiTaoWiFi();

  dht.begin();

  lcd.init();

  lcd.backlight();

  // FIX ADC ESP32
  analogSetAttenuation(ADC_11db);

  pinMode(NUT_MODE, INPUT_PULLUP);

  pinMode(NUT_QUAT, INPUT_PULLUP);

  pinMode(NUT_DEN, INPUT);

  pinMode(NUT_SERVO, INPUT);

  pinMode(QUAT, OUTPUT);

  pinMode(DEN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(NUT_MODE),
                  modeISR,
                  FALLING);

  attachInterrupt(digitalPinToInterrupt(NUT_QUAT),
                  quatISR,
                  FALLING);

  attachInterrupt(digitalPinToInterrupt(NUT_DEN),
                  denISR,
                  FALLING);

  attachInterrupt(digitalPinToInterrupt(NUT_SERVO),
                  servoISR,
                  FALLING);

  // ===== SERVO PWM =====
  ledc_timer_config_t timer = {

    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_16_BIT,
    .timer_num = LEDC_TIMER_0,
    .freq_hz = 50,
    .clk_cfg = LEDC_AUTO_CLK
  };

  ledc_timer_config(&timer);

  ledc_channel_config_t channel = {

    .gpio_num = SERVO_PIN,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel = LEDC_CHANNEL_0,
    .timer_sel = LEDC_TIMER_0,
    .duty = 0,
    .hpoint = 0
  };

  ledc_channel_config(&channel);

  duty_0 =
    map(1000, 0, 20000, 0, 65535);

  duty_180 =
    map(2000, 0, 20000, 0, 65535);

  servo_0();
}

// ================== LOOP ==================
void loop() {

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("WIFI ĐÃ SẴN SÀNG");

    // ===== DHT =====
    float nhietDo = dht.readTemperature();

    nhietDo = nhietDo - 2;

    float doAm = dht.readHumidity();

    doAm = doAm - 5;

    // ===== LỌC NHIỄU LDR =====
    long tongLux = 0;

    for (int i = 0; i < 10; i++) {

      tongLux += analogRead(CAMBIEN_SANG);

      delay(5);
    }

    int lux = tongLux / 10;

    if (isnan(nhietDo) || isnan(doAm)) {

      Serial.println("DHT ERROR");

      nhietDo = 0;

      doAm = 0;
    }

    Serial.printf("Sensor -> T:%.1f H:%.1f Lux:%d\n",
                  nhietDo,
                  doAm,
                  lux);

    // ===== TIME =====
    int gio = 0;

    int phut = 0;

    if (getLocalTime(&timeinfo)) {

      gio = timeinfo.tm_hour;

      phut = timeinfo.tm_min;
    }

    Serial.printf("Time -> %02d:%02d\n",
                  gio,
                  phut);

    // ===== FIREBASE =====
    if (millis() - lastFirebase > 3000) {

      docFirebase();

      lastFirebase = millis();
    }

    // ================= AUTO =================
    if (cheDo == "auto") {

      Serial.println(">>> MODE AUTO");

      // ===== QUẠT =====
      TTQuat = (nhietDo > nguongNhiet);

      // ===== AUTO ĐÈN =====
      static bool ledState = false;

      int nguongBat = nguongLux + 200;

      int nguongTat = nguongLux - 200;

      // tối -> bật
      if (lux > nguongBat) {

        ledState = true;
      }

      // sáng -> tắt
      if (lux < nguongTat) {

        ledState = false;
      }

      TTDen = ledState;

      // ===== AUTO CHO ĂN =====
      if (gio == gioChoAn &&
          phut == 0 &&
          giochay != gio) {

        Serial.println("AUTO FEED");

        servo_180();

        ServoChay = true;

        tgianbatS = millis();

        giochay = gio;
      }

      if (ServoChay &&
          millis() - tgianbatS >= 30000) {

        servo_0();

        ServoChay = false;
      }
    }

    // ================= MANUAL =================
    if (cheDo == "manual") {

      Serial.println(">>> MODE MANUAL");

      TTQuat = fb_quat;

      TTDen = fb_den;

      TTServo = fb_servo;

      // ===== FIX SERVO =====
      if (TTServo != lastServoState) {

        if (TTServo == 1) {

          servo_180();
        }
        else {

          servo_0();
        }

        lastServoState = TTServo;
      }
    }

    // ===== OUTPUT =====
    digitalWrite(QUAT, TTQuat);

    digitalWrite(DEN, TTDen);

    Serial.printf("Output -> Fan:%d Led:%d Servo:%d\n",
                  TTQuat,
                  TTDen,
                  TTServo);

    // ===== SEND FIREBASE =====
    if (millis() - lastSend > 3000) {

      guiFirebase(nhietDo,
                  doAm,
                  lux);

      lastSend = millis();
    }

    // ===== LCD =====
    if (millis() - lastLCD > 3000) {

      MH = !MH;

      if (MH == 0) {

        lcdSensor(nhietDo,
                  doAm,
                  lux);
      }
      else {

        lcdStatus();
      }

      lastLCD = millis();
    }

    Serial.println("------------------------");
  }

  // ================= OFFLINE =================
  else {

    Serial.println("WIFI CHƯA SẴN SÀNG");

    if (coNgatQuat) {

      trangThaiQuat = !trangThaiQuat;

      coNgatQuat = false;
    }

    if (coNgatDen) {

      trangThaiDen = !trangThaiDen;

      coNgatDen = false;
    }

    if (coNgatServo) {

      trangThaiServo = !trangThaiServo;

      if (trangThaiServo)
        servo_180();
      else
        servo_0();

      coNgatServo = false;
    }

    digitalWrite(QUAT, trangThaiQuat);

    digitalWrite(DEN, trangThaiDen);

    lcd.setCursor(0, 0);

    lcd.print("CHE DO OFFLINE");

    lcd.setCursor(0, 1);

    lcd.print("F:");

    lcd.print(trangThaiQuat ? "ON " : "OFF ");

    lcd.print("L:");

    lcd.print(trangThaiDen ? "ON " : "OFF ");

    lcd.print("S:");

    lcd.print(trangThaiServo ? "ON " : "OFF ");

    delay(500);
  }
}