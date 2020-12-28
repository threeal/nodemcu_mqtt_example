#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define WIFI_SSID "Kontrakan Bersama"
#define WIFI_PASS "ipkempat"

#define MQTT_SERVER "192.168.1.21"

WiFiClient wifiClient;
PubSubClient pubSubClient(wifiClient);

void setup() {
  Serial.begin(9600);
  Serial.println("Serial begin");

  setupWifi();

  pubSubClient.setServer(MQTT_SERVER, 1883);
  pubSubClient.setCallback(callback);
}

void setupWifi() {
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!pubSubClient.connected()) {
    Serial.print("Attempting MQTT connection... ");
    if (pubSubClient.connect("fan_subscriber")) {
      Serial.println("Connected");
      pubSubClient.subscribe("sensor/temperature");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(pubSubClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!pubSubClient.connected()) {
    reconnect();
  }

  pubSubClient.loop();

  delay(100);
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Received ");
  Serial.print((char*)message);
  Serial.print(" on ");
  Serial.println(topic);
}