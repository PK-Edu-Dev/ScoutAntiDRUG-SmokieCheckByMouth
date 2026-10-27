#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- SCREEN CONFIGURATION ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- HARDWARE PIN DEFINITIONS ---
#define SENSOR_PIN    34  // Analog Pin ตรวจวัด CO ในลมหายใจ
#define SENSOR_POWER  27  // จ่ายไฟเข้า Sensor
#define BUZZER_PIN    18  // แจ้งเตือนเสียง
#define LED_GREEN_PIN  4  // สถานะ ปกติ
#define LED_RED_PIN    5  // สถานะ ตรวจพบ CO สูง

// --- THRESHOLD CONFIGURATION (PPM) ---
// ค่า CO ปกติในลมหายใจคนไม่สูบบุหรี่: 0 - 6 ppm
// ค่า CO ในลมหายใจผู้สูบบุหรี่/บุหรี่ไฟฟ้า: > 10 ppm ขึ้นไป
const float CO_ALERT_PPM = 10.0; 

// --- FUNCTION TO CALCULATE CO PPM ---
float readCOPpm() {
    int rawAnalog = analogRead(SENSOR_PIN);
    
    // แปลงค่า ADC (0-4095) เป็น แรงดัน (0 - 3.3V)
    float voltage = (rawAnalog / 4095.0) * 3.3;

    // คำนวณค่า PPM แบบสเกลสมมติสำหรับ Simulation/Sensor (ปรับแต่งตาม Curve calibration จริงของ MQ-sensor)
    // สูตรประมาณค่า: (Voltage / 3.3) * Max_PPM_Scale
    float ppm = (voltage / 3.3) * 100.0; 

    return ppm;
}

void setup() {
    Serial.begin(115200);

    pinMode(SENSOR_POWER, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_RED_PIN, OUTPUT);

    digitalWrite(SENSOR_POWER, HIGH);

    Wire.begin(21, 22);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("[ERROR] OLED Allocation Failed!"));
        for (;;);
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(10, 20);
    display.println(F("ScoutAntiDRUG"));
    display.setCursor(10, 35);
    display.println(F("CO Breath Check..."));
    display.display();
    delay(2000);
}

void loop() {
    float co_ppm = readCOPpm();

    Serial.print(F("[DATA] CO Level: "));
    Serial.print(co_ppm);
    Serial.println(F(" ppm"));

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F(" CO BREATH CHECKER"));
    display.println(F("---------------------"));

    // แสดงผลค่า PPM ขนาดใหญ่
    display.setTextSize(2);
    display.setCursor(10, 22);
    display.print(co_ppm, 1);
    display.println(F(" PPM"));

    // การตัดสินผล
    if (co_ppm >= CO_ALERT_PPM) {
        digitalWrite(LED_RED_PIN, HIGH);
        digitalWrite(LED_GREEN_PIN, LOW);
        digitalWrite(BUZZER_PIN, HIGH);

        display.setTextSize(1);
        display.setCursor(0, 48);
        display.println(F("STATUS: SMOKE DETECTED"));
            } else {
        digitalWrite(LED_RED_PIN, LOW);
        digitalWrite(LED_GREEN_PIN, HIGH);
        digitalWrite(BUZZER_PIN, LOW);

        display.setTextSize(1);
        display.setCursor(0, 48);
        display.println(F("STATUS: CLEAN / NORMAL"));
    }

    display.display();
        delay(500);
}