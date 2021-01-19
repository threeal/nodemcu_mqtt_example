#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define WIFI_SSID "SSID"
#define WIFI_PASS "PASS"

#define MQTT_SERVER "192.168.1.1"

#define DHT11_PIN D1

WiFiClient wifiClient;
PubSubClient pubSubClient(wifiClient);

DHT dht(DHT11_PIN, DHT11);

void setup() {
  Serial.begin(9600);
  Serial.println("Serial begin");

  setupWifi();

  pubSubClient.setServer(MQTT_SERVER, 1883);

  dht.begin();
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
    if (pubSubClient.connect("temperature_publisher")) {
      Serial.println("Connected");
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

  float temperature = dht.readTemperature();

  if (!isnan(temperature)) {
    const char* topic = "sensor/temperature";

    char message[16];
    sprintf(message, "%f", temperature);

    pubSubClient.publish(topic, message);

    Serial.print("Published ");
    Serial.print(message);
    Serial.print(" on ");
    Serial.println(topic);
  }

  delay(1000);
}