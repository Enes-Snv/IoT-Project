#include <Wire.h>
#include <SFE_BMP180.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Infinix HOT 50 Pro+";
const char* password = "0123456789";

const char* mqtt_server = "192.168.237.29";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
SFE_BMP180 pressure;

const double SEA_LEVEL_PRESSURE = 1013.25;  // Standard sea level pressure (in hPa)

unsigned long lastMsg = 0;
const long interval = 3000;  // 3 seconds

void setup() {
  Serial.begin(9600);
  delay(10);

  if (!pressure.begin()) {
    while (1); // Stay here if sensor fails
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  client.setServer(mqtt_server, mqtt_port);
}

void reconnect() {
  while (!client.connected()) {
    if (!client.connect("D1MiniClient")) {
      delay(5000);
    }
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    char status;
    double T, P, a;

    status = pressure.startTemperature();
    if (status != 0) {
      delay(status);
      status = pressure.getTemperature(T);
      if (status != 0) {
        status = pressure.startPressure(3);
        if (status != 0) {
          delay(status);
          status = pressure.getPressure(P, T);
          if (status != 0) {
            a = pressure.altitude(P, SEA_LEVEL_PRESSURE);

            // Create JSON payloads
            char payload[128];
            char payload1[128];
            snprintf(payload, sizeof(payload),
                     "{\"pressure\":%.2f}", P);
            snprintf(payload1, sizeof(payload1),
                     "{\"Altitude\":%.2f}", a);

            client.publish("Pressure", payload, true);
            client.publish("Altitude", payload1, true);
          }
        }
      }
    }
  }
}