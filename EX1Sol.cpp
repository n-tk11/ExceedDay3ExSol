#include <Arduino.h>
#include <Bounce2.h>
#define BLUE 5
#define RED 12
#define BUTTON 27
TaskHandle_t TaskA = NULL;
Bounce debouncer = Bounce();
int cnt = 0;

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


void setup() {

  Serial.begin(115200);
  ledcSetup(0, 5000, 8);
  ledcAttachPin(BLUE, 0);

  debouncer.attach(BUTTON, INPUT_PULLUP);
  debouncer.interval(25);

  xTaskCreatePinnedToCore(Dim_LED, "Builtin_LED", 1000, NULL, 1, &TaskA, 0);
}

void loop() {
  debouncer.update();
  if (debouncer.fell()) {
    cnt++;
    Serial.println(cnt);
  }
}


