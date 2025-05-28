#include <Wire.h>
#include "HX711.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>  // Required for 16 MHz Adafruit Trinket
#endif

#define DT 2             // HX711 데이터 핀
#define SCK 3            // HX711 클럭 핀
#define SLAVE_ADDR 0x08  // I2C 슬레이브 주소
#define LED_PIN 4        // LED 핀 (원하면)
#define LED_COUNT 15     // How many NeoPixels are attached to the Arduino?

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

HX711 scale;
float calibration_factor = -102.0;
float weight = 0;

float delaytime = 1000;
bool led_on = false;

unsigned long previousMillis = 0;
const unsigned long interval = 200;  // 200ms마다 무게 갱신

bool ledOn = false;
unsigned long ledStartMillis = 0;

void setup() {

  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  strip.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  uint32_t white = strip.Color(255, 255, 255);
  strip.fill(white, 0, 15);
  strip.show();  // Turn OFF all pixels ASAP
  strip.setBrightness(255);
  strip.show();
  delay(3000);
  for (int i = 255; i > 0; i--) {
    strip.fill(strip.Color(i, i, i), 0, 15);
    strip.show();
    delay(10);
  }


  Serial.begin(9600);
  Wire.begin(SLAVE_ADDR);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

  scale.begin(DT, SCK);  // 🔹 여기에서 핀 설정
  scale.set_scale();
  scale.tare();
  Serial.println("영점 조정 완료!");
  scale.set_scale(calibration_factor);
  Serial.println("보정값 설정 완료!");

  Serial.println("슬레이브 시작, HX711 초기화 완료");
}

void loop() {
  unsigned long currentMillis = millis();

  // 주기적으로 무게 갱신
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    weight = scale.get_units(1);
  }

  if (led_on && currentMillis - ledStartMillis < delaytime && currentMillis - ledStartMillis < 100000) {
    float br = (1-((currentMillis - ledStartMillis) / delaytime)) * 255;
    Serial.println(br);
    strip.fill(strip.Color(br, br, br), 0, 15);
    strip.show();
  }


  if (led_on && currentMillis - ledStartMillis >= delaytime && currentMillis - ledStartMillis < 100000) {
    strip.clear();
    strip.show();
    Serial.println("led off");
    led_on = false;
  }
  delay(5);
}


void requestEvent() {

  ledStartMillis = millis();
  led_on = true;


  int latestWeight = (int)weight;
  Wire.write((latestWeight >> 8) & 0xFF);  // 상위 바이트
  Wire.write(latestWeight & 0xFF);         // 하위 바이트

  // 🔸 LED 켜짐 요청만 등록
}



void receiveEvent(int howMany) {
  if (howMany >= 2) {
    int high = Wire.read();
    int low = Wire.read();
    int value = (high << 8) | low;
    Serial.print("Received 2-byte value: ");
    Serial.println(value);
    delaytime = (float)value;
    Serial.print("delaytime is :");
    Serial.println(delaytime);
  }
}
