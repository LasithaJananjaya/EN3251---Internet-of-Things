#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define PERIOD 5000
StaticJsonDocument<2048> doc;
String data_out;

const char *SSID = "iPhone 7 plus";              //"Pixel 6a"                  // SSID of your WiFi
const char *PASSWORD = "hello_world";            //"pixelmcr6a"             //"206fde266242";       // Password of your WiFi
const char *mqqttBroker = "broker.hivemq.com";  // alternate hosts: test.mosquitto.org, broker.hivemq.com
const int mqttPort = 1883;
const char *mqttClientID = "200650U";   // CHANGE THIS acording to your group number
const char *POTTopic = "protocolpros";  // Topic for potentiometer (publish)

String ssid;
int32_t rssi;
uint8_t encryptionType;
uint8_t *bssid;
int32_t channel;
bool hidden;
int scanResult;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

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
  serializeJson(doc, data_out);
  mqttClient.publish(POTTopic, data_out.c_str());
  Serial.println("----------------------------------------data_out----------------------------------------");
  Serial.println(data_out);
  Serial.println("----------------------------------------Published----------------------------------------");
  doc = {};
  //doc.clear();
  Serial.println("----------------------------------------doc_clear----------------------------------------");
}

void setup() {
  Serial.begin(115200);
  mqttInit();
}

void loop() {
  //----------------------------------------Scanning----------------------------------------
  WiFi.mode(WIFI_AP_STA);
  Serial.println(F("Starting WiFi scan..."));
  scanResult = WiFi.scanNetworks(/*async=*/false, /*hidden=*/true);
  Serial.println("-----------------------------------Reached Tag 1-----------------------------------");
  //doc["networks"] = scanResult;

  if (scanResult == 0) {
    Serial.println(F("No networks found"));
  }

  else if (scanResult > 0) {
    Serial.printf(PSTR("%d networks found:\n"), scanResult);
    // Print unsorted scan results
    for (int8_t i = 0; i < scanResult; i++) {
      WiFi.getNetworkInfo(i, ssid, encryptionType, rssi, bssid, channel, hidden);

      char bssidStr[18];  // Create a char array to hold the BSSID as a string
      snprintf(bssidStr, sizeof(bssidStr), "%02X:%02X:%02X:%02X:%02X:%02X", bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
      doc[i]["bssid"] = bssidStr;
      doc[i]["rssi"] = rssi;
      //doc["ssid"] = ssid.c_str();
      Serial.printf(PSTR("  %02d: [CH %02d] [%02X:%02X:%02X:%02X:%02X:%02X] %ddBm %s\n"), i, channel, bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5], rssi, ssid.c_str());
      yield();
    }
  }

  else {
    Serial.printf(PSTR("WiFi scan error %d"), scanResult);
  }
  // Wait a bit before scanning again
  delay(PERIOD);


  //----------------------------------------Publishing----------------------------------------
  mqttLoop();
  sendValues();
}