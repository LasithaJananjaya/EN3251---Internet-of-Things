#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define POT_PIN A0
#define LED_1 D1
#define LED_2 D2
#define LED_3 D3
#define LED_4 D4

#define PERIOD 5000

const char *SSID = "AndroidAPF80D";               // SSID of your WiFi
const char *PASSWORD = "ptmp2405";       // Password of your WiFi
const char *mqqttBroker = "broker.emqx.io";  // alternate hosts: test.mosquitto.or, broker.hivemq.com
const int mqttPort = 1883;
const char *mqttClientID = "NodeMCU_protocolpros";  // CHANGE THIS acording to your group number
const char *POTTopic = "protocolpros/pot";          // Topic for potentiometer (publish)
const char *LEDTopic = "protocolpros/led/mcr";          // Topic for LED (subscribe)

WiFiClient espClient;
PubSubClient mqttClient(espClient);

unsigned long potTime;

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  char payloadCharArr[length];

  for (int i = 0; i < (int)length; i++) {
    payloadCharArr[i] = (char)payload[i];
  }
  String payloadStr = String(payloadCharArr).substring(0, length);

  if (strcmp(topic, LEDTopic) == 0) {
    // You can use the payloadStr to do something
    Serial.print("LED: ");
    Serial.println(payloadStr);

    if (payloadStr == "1") {
      digitalWrite(LED_1, HIGH);
      digitalWrite(LED_2, LOW);
      digitalWrite(LED_3, LOW);
      digitalWrite(LED_4, LOW);
    } else if (payloadStr == "2") {
      digitalWrite(LED_1, HIGH);
      digitalWrite(LED_2, HIGH);
      digitalWrite(LED_3, LOW);
      digitalWrite(LED_4, LOW);
    } else if (payloadStr == "3") {
      digitalWrite(LED_1, HIGH);
      digitalWrite(LED_2, HIGH);
      digitalWrite(LED_3, HIGH);
      digitalWrite(LED_4, LOW);
    } else {
      digitalWrite(LED_1, HIGH);
      digitalWrite(LED_2, HIGH);
      digitalWrite(LED_3, HIGH);
      digitalWrite(LED_4, HIGH);
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

void pinInit() {
  pinMode(POT_PIN, INPUT);
  potTime = millis();
}

void sendValues() {
  if (millis() - potTime > PERIOD) {
    int potValue = analogRead(POT_PIN);
    mqttClient.publish(POTTopic, String(potValue).c_str());
    potTime = millis();
    Serial.print("POT: ");
    Serial.println(potValue);
  }
}

void setup() {
  Serial.begin(115200);
  mqttInit();
  pinInit();
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);
  
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
  digitalWrite(LED_3, LOW);
  digitalWrite(LED_4, LOW);
}

void loop() {
  mqttLoop();
  sendValues();
}