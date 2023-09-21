#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

StaticJsonDocument<200> sensor_out;
String data_out;

#define POT_PIN 34
#define LDR_PIN 35

#define PERIOD 5000

const char *SSID = "Pixel 6a";               // SSID of your WiFi
const char *PASSWORD = "pixelmcr6a"; //"206fde266242";       // Password of your WiFi
const char *mqqttBroker = "test.mosquitto.org";  // alternate hosts: test.mosquitto.or, broker.hivemq.com
const int mqttPort = 1883;
const char *mqttClientID = "200650U";  // CHANGE THIS acording to your group number
const char *POTTopic = "protocolpros";          // Topic for potentiometer (publish)

WiFiClient espClient;
PubSubClient mqttClient(espClient);

unsigned long potTime;

String ldrRange(int value) {
  // 300-3300
  if (value <= 1000) {
    return "Low";
  } else if (value <= 2000) {
    return "Medium";
  } else {
    return "High";
  }
}

String potRange(int value) {
  // 300-3300
  if (value <= 1500) {
    return "Low";
  } else if (value <= 3000) {
    return "Medium";
  } else {
    return "High";
  }
}

void mqttInit() {
  mqttClient.setServer(mqqttBroker, mqttPort);
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
    if (mqttClient.connect(mqttClientID)) {
      Serial.println("connected");
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
  if (millis() - potTime > PERIOD) {
    int potValue = analogRead(POT_PIN);
    int ldrValue = analogRead(LDR_PIN);

    sensor_out["POT"] = potRange(potValue);
    sensor_out["LDR"] = ldrRange(ldrValue);

    serializeJson(sensor_out, data_out);

    //mqttClient.publish(POTTopic, (String(potValue) + " - " + String(ldrValue)).c_str());
    mqttClient.publish(POTTopic, data_out.c_str());
    potTime = millis();
    Serial.print("POT: ");
    Serial.print(potValue);
    Serial.print(" - ");
    Serial.print("LDR: ");
    Serial.println(ldrValue);
    Serial.println(data_out);

    sensor_out.clear();
    data_out = "";
  }
}

void setup() {
  Serial.begin(115200);
  mqttInit();
  potTime = millis();
  pinMode(POT_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
}

void loop() {
  mqttLoop();
  sendValues();
}