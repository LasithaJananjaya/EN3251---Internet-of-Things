#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define RED D6
#define GREEN D7
#define YELLOW D8

#define PERIOD 5000

unsigned long readTime;

const char *SSID = "Pixel 6a";//"AndroidAPF80D";              // SSID of your WiFi
const char *PASSWORD = "pixelmcr6a";//"ptmp2405";               // Password of your WiFi
const char *mqqttBroker = "test.mosquitto.org";  // alternate hosts: test.mosquitto.or, broker.hivemq.com
const int mqttPort = 1883;
const char *mqttClientID = "NodeMCU_mcr";  // CHANGE THIS acording to your group number
const char *LEDTopic = "protocolpros";     // Topic for LED (subscribe)

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  char payloadCharArr[length];

  for (int i = 0; i < (int)length; i++) {
    payloadCharArr[i] = (char)payload[i];
  }
  String payloadStr = String(payloadCharArr).substring(0, length);

  if (strcmp(topic, LEDTopic) == 0) {
    // You can use the payloadStr to do something
    //Serial.print("LED: ");
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
    }
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
    if (mqttClient.connect(mqttClientID)) {
      Serial.println("connected");
      mqttClient.subscribe(LEDTopic);  // Subscribe to LEDTopic
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

void setup() {
  Serial.begin(115200);
  mqttInit();
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(YELLOW, OUTPUT);

  digitalWrite(RED, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(YELLOW, LOW);
}

void loop() {
  mqttLoop();
}