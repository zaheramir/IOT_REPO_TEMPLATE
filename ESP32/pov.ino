#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <FastLED.h>

#define LED_PIN 27
#define NUM_LEDS 40
#define BRIGHTNESS 120

CRGB leds[NUM_LEDS];

#define NUM_COLS 24
uint8_t image[NUM_COLS][NUM_LEDS];

Adafruit_MPU6050 mpu;
float angle = 0.0f;
unsigned long lastTime = 0;

#define ANGLE_MIN -60.0f
#define ANGLE_MAX  60.0f

void buildHeart();
int mapAngleToColumn(float a);
void drawColumn(int col);

void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin();

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  delay(1000);

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  if (!mpu.begin()) {
    while (1) {
      fill_solid(leds, NUM_LEDS, CRGB::Red);
      FastLED.show();
      delay(200);
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
      delay(200);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  lastTime = millis();
  buildHeart();
}

void loop() {
  sensors_event_t acc, gyro, temp;
  mpu.getEvent(&acc, &gyro, &temp);

  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0f;
  lastTime = now;

  float gyroZ_deg = gyro.gyro.z * 57.2958f;

  angle += gyroZ_deg * dt;
  angle *= 0.98f;

  int col = mapAngleToColumn(angle);
  drawColumn(col);

  Serial.print(angle);
  Serial.print(" ");
  Serial.println(col);
}

int mapAngleToColumn(float a) {
  if (a < ANGLE_MIN || a > ANGLE_MAX) {
    return -1;
  }
  long A = (long)(a * 100.0f);
  long A_MIN = (long)(ANGLE_MIN * 100.0f);
  long A_MAX = (long)(ANGLE_MAX * 100.0f);
  return map(A, A_MIN, A_MAX, 0, NUM_COLS - 1);
}

void drawColumn(int col) {
  if (col < 0 || col >= NUM_COLS) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  } else {
    for (int y = 0; y < NUM_LEDS; y++) {
      uint8_t v = image[col][y];
      if (v == 0) {
        leds[y] = CRGB::White;
      } else if (v == 1) {
        leds[y] = CRGB(255, 200, 0);
      } else {
        leds[y] = CRGB::Black;
      }
    }
  }
  FastLED.show();
}

void buildHeart() {
  memset(image, 0, sizeof(image));

  const int cx = 12;
  const int cy = 20;
  const int r2 = 110;

  for (int x = 0; x < NUM_COLS; x++) {
    for (int y = 0; y < NUM_LEDS; y++) {
      uint8_t v = 0;

      int dx = x - cx;
      int dy = y - cy;

      long dist2 = dx * dx + (long)(dy * dy) / 2;

      if (dist2 <= r2) {
        v = 1;

        if (x >= cx - 5 && x <= cx - 3 && y >= cy - 6 && y <= cy - 4) {
          v = 2;
        } else if (x >= cx + 3 && x <= cx + 5 && y >= cy - 6 && y <= cy - 4) {
          v = 2;
        }

        int mouthY = cy + 4;
        int dy_m = y - mouthY;

        if (dy_m >= 0 && dy_m <= 3) {
          int adx = abs(dx);
          if (dy_m == 0 && adx >= 2 && adx <= 6) v = 2;
          else if (dy_m == 1 && adx >= 2 && adx <= 5) v = 2;
          else if (dy_m == 2 && adx >= 2 && adx <= 4) v = 2;
          else if (dy_m == 3 && adx >= 2 && adx <= 3) v = 2;
        }
      }

      image[x][y] = v;
    }
  }
}
