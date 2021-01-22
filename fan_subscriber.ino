#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define WIFI_SSID "SSID"
#define WIFI_PASS "PASS"

#define MQTT_SERVER "192.168.1.1"

#define L9110_INA_PIN D1
#define L9110_INB_PIN D2

WiFiClient wifiClient;
PubSubClient pubSubClient(wifiClient);

int people_count = 2;
float temperature = 30;

long prev;
long counter;

void setup() {
  Serial.begin(9600);
  Serial.println("Serial begin");

  setupWifi();

  pubSubClient.setServer(MQTT_SERVER, 1883);
  pubSubClient.setCallback(callback);

  pinMode(L9110_INA_PIN, OUTPUT);
  pinMode(L9110_INB_PIN, OUTPUT);

  digitalWrite(L9110_INA_PIN, LOW);
  digitalWrite(L9110_INB_PIN, LOW);

  prev = millis();
  counter = 0;
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
      pubSubClient.subscribe("sensor/people_count");
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

  float intensity = (people_count / 3) + (temperature - 30) / 15;
  if (intensity < 0) {
    intensity = 0;
  } else if (intensity > 1) {
    intensity = 1;
  }

  long now = millis();

  counter += now - prev;
  if (counter < (float)200 * intensity) {
    digitalWrite(L9110_INA_PIN, HIGH);
  } else if (counter < 200) {
    digitalWrite(L9110_INA_PIN, LOW);
  } else {
    digitalWrite(L9110_INA_PIN, HIGH);

    const char* topic = "sensor/fan_speed";

    char message[16];
    sprintf(message, "%f", intensity);

    pubSubClient.publish(topic, message);

    Serial.print("Published ");
    Serial.print(message);
    Serial.print(" on ");
    Serial.println(topic);

    counter -= 200;
  }

  prev = now;
}

void callback(char* topic, byte* message, unsigned int length) {
  String str;

  Serial.print("Received ");
  for (unsigned int i = 0; i < length; ++i) {
    Serial.print((char)message[i]);
    str += (char)message[i];
  }
  Serial.print(" on ");
  Serial.println(topic);

  if (String(topic) == "sensor/temperature") {
    temperature = atof(str.c_str());
  } else if (String(topic) == "sensor/people_count") {
    people_count = atoi(str.c_str());
  }
}