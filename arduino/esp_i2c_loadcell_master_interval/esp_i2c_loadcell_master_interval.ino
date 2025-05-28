#include <Wire.h>

#define BUTTON_PIN 15  // 버튼 연결 핀
#define SDA_PIN 19
#define SCL_PIN 23

const int potpin = 15;

bool lastButtonState = HIGH;

int interval = 1000;
int thisstep = 0;

unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(potpin, INPUT);
  Serial.println("ESP32 마스터 시작");
}

void loop() {
  unsigned long currentMillis = millis();
  interval = analogRead(potpin) + 100;

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    Serial.print("interval is ");
    Serial.print(interval);
    Serial.print(" , and this step is ");
    Serial.println(thisstep);

    byte address = 0x08 + thisstep;  // 주소 계산

    Wire.beginTransmission(address);
    Wire.write((interval >> 8) & 0xFF);  // 상위 바이트
    Wire.write(interval & 0xFF);         // 하위 바이트
    Wire.endTransmission();

    Serial.print("send data to 0x0");
    Serial.println(address, HEX);  // 16진수로 출력

    // 슬레이브에 요청 (고정된 주소에서만 받음)
    Wire.requestFrom(address, 2);  // 응답은 항상 0x08에서 받는 것으로 가정
    if (Wire.available() == 2) {
      int high = Wire.read();
      int low = Wire.read();
      int weight = (high << 8) | low;
      Serial.print("Received weight Value: ");
      Serial.println(weight);
    }

    thisstep++;
    if (thisstep > 7) thisstep = 0;  // 0~7 반복
  }
}
