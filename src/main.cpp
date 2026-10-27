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

// --- CONFIGURATION THRESHOLD ---
const int DANGER_THRESHOLD = 400; // ค่าปรับเปลี่ยนตามการ Calibrate จริง

void setup() {
    Serial.begin(115200);

    // กำหนด Pin Mode
    pinMode(SENSOR_POWER, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_RED_PIN, OUTPUT);

    // จ่ายไฟเข้า Sensor
    digitalWrite(SENSOR_POWER, HIGH);

    // เริ่มต้นใช้งาน I2C (SDA=21, SCL=22)
    Wire.begin(21, 22);

    // เริ่มต้นใช้งานจอ OLED (Address 0x3C)
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("[ERROR] OLED SSD1306 Allocation Failed!"));
        for (;;);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.println(F("  ScoutAntiDRUG System"));
    display.println(F(" Status: Initializing..."));
    display.display();
    delay(2000);
}

void loop() {
    // อ่านค่า Analog จากเซนเซอร์
    int sensorRawValue = analogRead(SENSOR_PIN);

    Serial.print(F("[DATA] CO Level: "));
    Serial.println(sensorRawValue);

    // ล้างหน้าจอเพื่ออัปเดตข้อมูลใหม่
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println(F(" ScoutAntiDRUG Monitor"));
    display.println(F("----------------------"));
    
    display.setCursor(0, 20);
    display.print(F("CO Value: "));
    display.println(sensorRawValue);

    // Logic การตรวจสอบระดับอันตราย
    if (sensorRawValue >= DANGER_THRESHOLD) {
        // สภาวะอันตราย (Danger)
        digitalWrite(LED_RED_PIN, HIGH);
        digitalWrite(LED_GREEN_PIN, LOW);
        digitalWrite(BUZZER_PIN, HIGH);

        display.setCursor(0, 40);
        display.setTextSize(2);
        display.println(F("DANGER!!"));
    } else {
        // สภาวะปกติ (Safe)
        digitalWrite(LED_RED_PIN, LOW);
        digitalWrite(LED_GREEN_PIN, HIGH);
        digitalWrite(BUZZER_PIN, LOW);

        display.setCursor(0, 40);
        display.setTextSize(2);
        display.println(F("NORMAL"));
    }

    display.display();
    delay(500); // หน่วงเวลาอ่านค่าทุก 0.5 วินาที
}
