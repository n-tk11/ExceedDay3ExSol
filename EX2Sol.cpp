#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define BLUE 5

const String GetDelayUrl = "https://exceed-hardware-stamp465.koyeb.app/leddelay";
const char* ssid = "TESTHOS";
const char* password = "waer3821";

int myDelay;

void Connect_Wifi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("OK! IP=");
  Serial.println(WiFi.localIP());
}


void GET_delay() {
  DynamicJsonDocument doc(2048);
  const String url = GetDelayUrl;
  HTTPClient http;
  http.begin(url);
  int httpResponseCode = http.GET();

  if (httpResponseCode >= 200 && httpResponseCode < 300) {
    Serial.print("HTTP ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    deserializeJson(doc, payload);

    myDelay = doc["value"].as<int>();
    Serial.println(myDelay);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
}

void HeartBeat(void* param) {
  while (1) {
    digitalWrite(BLUE, 1);
    vTaskDelay(myDelay / portTICK_PERIOD_MS);
    digitalWrite(BLUE, 0);
    vTaskDelay(myDelay / portTICK_PERIOD_MS);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(BLUE, OUTPUT);
  Connect_Wifi();
  xTaskCreatePinnedToCore(HeartBeat, "HeartBeat", 1000, NULL, 1, NULL, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  GET_delay();
  vTaskDelay(5000 / portTICK_PERIOD_MS);
}