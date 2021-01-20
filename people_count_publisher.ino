#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define WIFI_SSID "SSID"
#define WIFI_PASS "PASS"

#define MQTT_SERVER "192.168.1.1"

#define OUTSIDE_TRIGGER_PIN D1
#define OUTSIDE_ECHO_PIN D2
#define INSIDE_TRIGGER_PIN D3
#define INSIDE_ECHO_PIN D4

WiFiClient wifiClient;
PubSubClient pubSubClient(wifiClient);

int peopleCount = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Serial begin");

  setupWifi();

  pubSubClient.setServer(MQTT_SERVER, 1883);

  pinMode(OUTSIDE_TRIGGER_PIN, OUTPUT);
  pinMode(OUTSIDE_ECHO_PIN, INPUT);
  pinMode(INSIDE_TRIGGER_PIN, OUTPUT);
  pinMode(INSIDE_ECHO_PIN, INPUT);
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
    if (pubSubClient.connect("people_count_publisher")) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(pubSubClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

int updatePeopleCount(int newPeopleCount) {
  peopleCount = newPeopleCount;

  while (!pubSubClient.connected()) {
    reconnect();
  }

  pubSubClient.loop();

  const char* topic = "sensor/people_count";

  char message[16];
  sprintf(message, "%d", peopleCount);

  pubSubClient.publish(topic, message);

  Serial.print("Published ");
  Serial.print(message);
  Serial.print(" on ");
  Serial.println(topic);

  return peopleCount;
}

void loop() {
  if (checkOutside()) {
    Serial.println("Waiting someone to come in");
    while (!checkInside()) {
      delay(5);
    }

    updatePeopleCount(peopleCount + 1);

    Serial.print("Someone came in, ");
    Serial.print(peopleCount);
    Serial.println(" people inside");

    delay(1000);
  } else if (checkInside()) {
    Serial.println("Waiting someone to come out");
    while (!checkOutside()) {
      delay(5);
    }

    updatePeopleCount(peopleCount - 1);

    Serial.print("Someone came out, ");
    Serial.print(peopleCount);
    Serial.println(" people inside");

    delay(1000);
  }
}

bool checkDistance(int triggerPin, int echoPin) {
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  float distance = (duration / 2) / 29.1;

  return distance >= 5 && distance <= 20;
}

bool checkOutside() {
  return checkDistance(OUTSIDE_TRIGGER_PIN, OUTSIDE_ECHO_PIN);
}

bool checkInside() {
  return checkDistance(INSIDE_TRIGGER_PIN, INSIDE_ECHO_PIN);
}