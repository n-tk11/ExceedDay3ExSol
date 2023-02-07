#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wifi.h>
#include <Bounce2.h>
#include <string.h>

#define BLUE 5
#define RED 33
#define BUTTON 27

//const int _size = 2 * JSON_OBJECT_SIZE(20);
//StaticJsonDocument<_size> JSONPost;
//StaticJsonDocument<_size> JSONGet;

const String BaseUrl = "https://exceed-hardware-stamp465.koyeb.app";
const char* ssid = "TESTHOS";
const char* password = "waer3821";

int timeout = 5;
int buttonState = 0;
int a, b, c;
String op, questionId;

Bounce debouncer = Bounce();

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

void Dim_LED(void* param) {
  while (1) {
    for (int i = 0; i < 255; i++) {
      ledcWrite(0, i);
      vTaskDelay(5 / portTICK_PERIOD_MS);
    }
    for (int i = 255; i > 0; i--) {
      ledcWrite(0, i);
      vTaskDelay(5 / portTICK_PERIOD_MS);
    }
  }
}



void saving_LED(void* param) {
  while (1) {
    debouncer.update();
    if (debouncer.fell()) {
      if (buttonState == 0) {
        digitalWrite(RED, 1);
        buttonState = 1;
        timeout = 5;
      }
      else {
        digitalWrite(RED, 0);
        buttonState = 0;
      }
    }

  }
}

void calculation() {
  if (op == "+")
    c = a + b;
  if (op == "-")
    c = a - b;
  if (op == "*")
    c = a * b;
}

void GET_question() {
  DynamicJsonDocument JSONGet(2048);
  const String url = BaseUrl + "/question";
  HTTPClient http;
  http.begin(url);
  int httpResponseCode = http.GET();

  if (httpResponseCode >= 200 && httpResponseCode < 300) {
    Serial.print("HTTP ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    deserializeJson(JSONGet, payload);

    a = JSONGet["a"].as<int>();
    b = JSONGet["b"].as<int>();
    op = (const char*)JSONGet["op"];
    questionId = (const char*)JSONGet["questionId"];

    Serial.println("GET Result");
    Serial.printf("a : %d\n", a);
    Serial.printf("b : %d\n", b);
    Serial.printf("op : %s\n", op);
    Serial.printf("questionId : %s\n", questionId);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

void Post_ans() {
  DynamicJsonDocument JSONPost(2048);
  String json;
  JSONPost["questionId"] = questionId;
  JSONPost["result"] = c;

  serializeJson(JSONPost, json);

  const String url = BaseUrl + "/answer";
  HTTPClient http;
  http.begin(url);
  //int httpResponseCode = http.POST();

  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(json);
  if (httpResponseCode >= 200 && httpResponseCode < 300) {
    Serial.print("HTTP ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

void calculator(void* param) {
  while (1) {
    GET_question();
    calculation();
    Serial.printf("result: %d\n", c);
    Serial.printf("questionId: %s\n", questionId);
    Post_ans();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  ledcSetup(0, 5000, 8);
  ledcAttachPin(BLUE, 0);

  pinMode(RED, OUTPUT);

  debouncer.attach(BUTTON, INPUT_PULLUP);
  debouncer.interval(25);
  Connect_Wifi();

  xTaskCreatePinnedToCore(Dim_LED, "Dim", 1024, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(saving_LED, "saving", 1024, NULL, 0, NULL, 0);
  xTaskCreatePinnedToCore(calculator, "calculate", 10 * 1024, NULL, 2, NULL, 1);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (buttonState == 1) {
    Serial.println(timeout);
    if (timeout == 0 && buttonState == 1) {
      digitalWrite(RED, 0);
      buttonState = 0;
    }
    timeout--;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}