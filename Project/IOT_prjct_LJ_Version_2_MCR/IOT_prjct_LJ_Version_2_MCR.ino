#include "ZMPT101B.h"
#include "ACS712.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

StaticJsonDocument<200> sensor_out;
String data_out;

#define PERIOD 5000
unsigned long sendTime;
unsigned long readTime;

const char *SSID = "gevidu";               // SSID of your WiFi
const char *PASSWORD = "123456789";        //"206fde266242";       // Password of your WiFi
const char *mqqttBroker = "31.220.81.30";  //"test.mosquitto.org"; alternate hosts: test.mosquitto.or, broker.hivemq.com
const int mqttPort = 1883;
const char *mqttClientID = "ProtocolPros_1";  // CHANGE THIS acording to your group number
const char *pubTopic = "Protocol_pros";       // Topic for publish
const char *subTopic = "protocolpros";     // Topic for subscribe

WiFiClient espClient;
PubSubClient mqttClient(espClient);

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

ZMPT101B voltageSensor(35);
ACS712 currentSensor(ACS712_20A, 34);

int mVperAmp = 100;
double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;

float P = 0;  //power
float U = 0;  //voltage
float I = 0;  //current
long dt = 0;
float CulmPwh = 0;
float units = 0;
long changeScreen = 0;
float lastSample = 0;

unsigned long lasttime = 0;
long ScreenSelect = 0;

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  char payloadCharArr[length];

  for (int i = 0; i < (int)length; i++) {
    payloadCharArr[i] = (char)payload[i];
  }
  String payloadStr = String(payloadCharArr).substring(0, length);

  if (strcmp(topic, subTopic) == 0) {
    // You can use the payloadStr to do something
    Serial.println(payloadStr);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payloadStr);


    const char *POT_ = doc["POT"];
    Serial.print(POT_);
    Serial.print(" - ");
    const char *LDR_ = doc["LDR"];
    Serial.println(LDR_);

    readTime = millis();

    String POT = String(POT_);
    String LDR = String(LDR_);

/*
    if ((POT == "Low" && LDR == "Low") || (POT == "Low" && LDR == "Medium") || (POT == "Medium" && LDR == "Low")) {
      while (millis() - readTime < PERIOD) {
        Serial.print(".");
        digitalWrite(RED, HIGH);
        digitalWrite(GREEN, HIGH);
        digitalWrite(YELLOW, HIGH);
        delay(100);
        digitalWrite(RED, LOW);
        digitalWrite(GREEN, LOW);
        digitalWrite(YELLOW, LOW);
        delay(100);
      }
    } else if (((LDR == "Medium") && (POT == "Medium")) || ((LDR == "High") && (POT == "Low")) || ((LDR == "Low") && (POT == "High"))) {
      while (millis() - readTime < PERIOD) {
        Serial.print(".");
        digitalWrite(RED, LOW);
        digitalWrite(GREEN, HIGH);
        digitalWrite(YELLOW, HIGH);
      }

    } else if (((LDR == "High") && (POT == "Medium")) || ((LDR == "Medium") && (POT == "High")) || ((LDR == "High") && (POT == "High"))) {
      while (millis() - readTime < PERIOD) {
        Serial.print(".");
        digitalWrite(RED, HIGH);
        digitalWrite(GREEN, HIGH);
        digitalWrite(YELLOW, HIGH);
      }
    } */
  }
}

void mqttInit() {
  mqttClient.setServer(mqqttBroker, mqttPort);
  mqttClient.setCallback(mqttCallback);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqttLoop() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect(mqttClientID, "smartplug_tx", "protocolpros_tx")) {
      Serial.println("connected");
      mqttClient.subscribe(subTopic);  // Subscribe to subTopic
      Serial.println("MQTT subscribed");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
  mqttClient.loop();
}

void sendValues() {
  if (millis() - sendTime > PERIOD) {

    sensor_out["Voltage"] = String(U);
    sensor_out["Current"] = String(I);

    serializeJson(sensor_out, data_out);

    mqttClient.publish(pubTopic, data_out.c_str());
    sendTime = millis();
    Serial.println(data_out);

    sensor_out.clear();
    data_out = "";
  }
}

void setup() {
  Serial.begin(115200);

  mqttInit();
  sendTime = millis();

  delay(100);
  voltageSensor.setSensitivity(0.0025);
  voltageSensor.setZeroPoint(2333);

  currentSensor.setZeroPoint(3133);
  currentSensor.setSensitivity(0.15);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  // Clear the buffer
  display.clearDisplay();
  display.display();

  //CalibCurrent();
  //CalibVoltage();
}


void loop() {

  U = voltageSensor.getVoltageAC();
  if (U < 55) {
    U = 0;
    CulmPwh = 0;
  }

  I = currentSensor.getCurrentAC();
  dt = micros() - lastSample;

  if (I < 0.15) {
    I = 0;
    CulmPwh = 0;
  }

  P = U * I;

  CulmPwh = CulmPwh + P * (dt / 3600);  ///uWh
  units = CulmPwh / 1000;

  if (millis() - changeScreen > 5000) {
    ScreenSelect += 1;
    changeScreen = millis();
  }

  if (millis() - lasttime > 500) {
    if ((ScreenSelect % 4) == 0) { displayVoltCurrent(); }  //Volts and Current

    else if ((ScreenSelect % 4) == 1) {
      displayInstPower();
    }  //Instantaenous Power

    else if ((ScreenSelect % 4) == 2) { displayEnergy(); }  //Energy

    else if ((ScreenSelect % 4) == 3) {
      displayUnits();
    }  //Units
  }
  lastSample = micros();

  //sending the values
  mqttLoop();
  sendValues();
}


float getVPP() {
  float result;
  int readValue;        // value read from the sensor
  int maxValue = 0;     // store max value here
  int minValue = 4096;  // store min value here ESP32 ADC resolution

  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000)  //sample for 1 Sec
  {
    readValue = analogRead(34);
    // see if you have a new maxValue
    if (readValue > maxValue) {
      /*record the maximum sensor value*/
      maxValue = readValue;
    }
    if (readValue < minValue) {
      /*record the minimum sensor value*/
      minValue = readValue;
    }
  }

  // Subtract min from max
  result = ((maxValue - minValue) * 3.3) / 4096.0;  //ESP32 ADC resolution 4096

  return result;
}

void displayVoltCurrent() {
  display.clearDisplay();
  display.setTextColor(WHITE);

  display.setTextSize(3);
  displayCenter(String(U) + "V", 3);

  display.setTextSize(3);
  displayCenter(String(I) + "A", 33);
  display.display();
  lasttime = millis();
}

void displayInstPower() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  displayCenter("Power", 3);
  display.setTextSize(3);

  if (P > 1000) {
    displayCenter(String(P / 1000) + "kW", 30);
  } else {
    displayCenter(String(P) + "W", 30);
  }

  display.display();
  lasttime = millis();
}

void displayEnergy() {
  display.clearDisplay();
  display.setTextColor(WHITE);

  if (CulmPwh > 1000000000) {
    display.setTextSize(2);
    displayCenter("Energy kWh", 3);
    display.setTextSize(3);
    displayCenter(String(CulmPwh / 1000000000), 30);
  } else if (CulmPwh < 1000000000 && CulmPwh > 1000000) {
    display.setTextSize(2);
    displayCenter("Energy Wh", 3);
    display.setTextSize(3);
    displayCenter(String(CulmPwh / 1000000), 30);
  } else if (CulmPwh < 1000000 && CulmPwh > 1000) {
    display.setTextSize(2);
    displayCenter("Energy mWh", 3);
    display.setTextSize(3);
    displayCenter(String(CulmPwh / 1000), 30);
  } else {
    display.setTextSize(2);
    displayCenter("Energy uWh", 3);
    display.setTextSize(3);
    displayCenter(String(CulmPwh), 30);
  }
  display.display();
  lasttime = millis();
}

void displayUnits() {
  display.clearDisplay();
  display.setTextColor(WHITE);

  if (units > 1000000) {
    display.setTextSize(2);
    displayCenter("Units", 3);
    display.setTextSize(3);
    displayCenter(String(units / 1000000), 30);
  } else if (units < 1000000 && units > 1000) {
    display.setTextSize(2);
    displayCenter("MilliUnits", 3);
    display.setTextSize(3);
    displayCenter(String(units / 1000), 30);
  } else {
    display.setTextSize(2);
    displayCenter("MicroUnits", 3);
    display.setTextSize(3);
    displayCenter(String(units), 30);
  }
  display.display();
  lasttime = millis();
}

void CalibCurrent() {
  while (1) {
    currentSensor.calibrate();
    Serial.print("Zero Point Current :");
    Serial.println(currentSensor.getZeroPoint());
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Current Zero Point :");
    display.setCursor(0, 20);
    display.setTextSize(2);
    display.print(currentSensor.getZeroPoint());
    display.display();
    delay(500);
  }
}

void CalibVoltage() {
  while (1) {
    voltageSensor.calibrate();
    Serial.print("Zero Point Voltage :");
    Serial.println(voltageSensor.getZeroPoint());
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Voltage Zero Point :");
    display.setCursor(0, 20);
    display.setTextSize(2);
    display.print(voltageSensor.getZeroPoint());
    display.display();
    delay(500);
  }
}

void displayCenter(String text, int line) {
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);
  // display on horizontal center
  display.setCursor((SCREEN_WIDTH - width) / 2, line);
  display.println(text);  // text to display
  display.display();
}
