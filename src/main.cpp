#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- HARDWARE PIN DEFINITIONS ---
#define SENSOR_PIN    34  // Analog Input ตรวจวัด CO
#define SENSOR_POWER  27  // VCC Sensor
#define BUZZER_PIN    18  // Buzzer Speaker
#define LED_GREEN_PIN  4  // Status Green LED
#define LED_RED_PIN    5  // Status Danger/Warning LED
#define BUTTON_PIN    14  // ปุ่มกดเริ่มตรวจวัด (ต่อลง GND)

// --- SYSTEM STATE & TIMING ---
bool isTesting = false;
unsigned long testStartTime = 0;
const unsigned long TEST_DURATION_MS = 300000; // 5 นาที (300,000 ms)

float readCOPpm() {
    int rawAnalog = analogRead(SENSOR_PIN);
    float voltage = (rawAnalog / 4095.0) * 3.3;
    float ppm = (voltage / 3.3) * 100.0;
    return ppm;
}

void setup() {
    Serial.begin(115200);

    pinMode(SENSOR_POWER, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP); // ปุ่มกด ACTIVE LOW

    digitalWrite(SENSOR_POWER, LOW); // พักการจ่ายไฟเซนเซอร์ตอน IDLE

    Wire.begin(21, 22);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        for (;;);
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(15, 20);
    display.println(F("ScoutAntiDRUG"));
    display.setCursor(10, 35);
    display.println(F("eCO Breath Analysis"));
    display.display();
    delay(2000);
}

void loop() {
    // --- เช็กการกดปุ่มเพื่อเริ่มตรวจวัด ---
    if (!isTesting && digitalRead(BUTTON_PIN) == LOW) {
        delay(50); // Debounce
        if (digitalRead(BUTTON_PIN) == LOW) {
            isTesting = true;
            testStartTime = millis();
            digitalWrite(SENSOR_POWER, HIGH); // จ่ายไฟเซนเซอร์
        }
    }

    if (!isTesting) {
        // --- IDLE STATE (พักเครื่อง รอรับคำสั่ง) ---
        digitalWrite(LED_GREEN_PIN, LOW);
        digitalWrite(LED_RED_PIN, LOW);
        noTone(BUZZER_PIN);

        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 5);
        display.println(F(" ScoutAntiDRUG eCO"));
        display.println(F("---------------------"));
        display.setCursor(10, 30);
        display.println(F("[ READY TO CHECK ]"));
        display.setCursor(5, 48);
        display.println(F("Press Button to Start"));
        display.display();
        delay(100);

    } else {
        // --- TESTING STATE (ตรวจวัด 5 นาที) ---
        unsigned long elapsedTime = millis() - testStartTime;
        
        // เช็กว่าครบ 5 นาทีหรือยัง
        if (elapsedTime >= TEST_DURATION_MS) {
            isTesting = false;
            digitalWrite(SENSOR_POWER, LOW); // ปิดไฟเซนเซอร์
            return;
        }

        // คำนวณเวลาถอยหลัง (วินาที)
        unsigned long remainingSec = (TEST_DURATION_MS - elapsedTime) / 1000;
        unsigned int mins = remainingSec / 60;
        unsigned int secs = remainingSec % 60;

        float co_ppm = readCOPpm();

        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print(F(" TESTING... "));
        
        // โชว์เวลาถอยหลังมุมบนขวา
        if (mins < 10) display.print(F("0"));
        display.print(mins);
        display.print(F(":"));
        if (secs < 10) display.print(F("0"));
        display.println(secs);
        
        display.println(F("---------------------"));

        // แสดงค่า PPM
        display.setTextSize(2);
        display.setCursor(10, 20);
        display.print(co_ppm, 1);
        display.println(F(" PPM"));

        display.setTextSize(1);
        display.setCursor(0, 48);

        // --- INTERPRETATION LOGIC (4 ระดับ) ---
        if (co_ppm <= 6.0) {
            digitalWrite(LED_GREEN_PIN, HIGH);
            digitalWrite(LED_RED_PIN, LOW);
            noTone(BUZZER_PIN);
            display.println(F("STATUS: CLEAN (NORMAL)"));

        } else if (co_ppm <= 10.0) {
            digitalWrite(LED_GREEN_PIN, HIGH);
            digitalWrite(LED_RED_PIN, HIGH);
            noTone(BUZZER_PIN);
            display.println(F("STATUS: LIGHT SMOKER"));

        } else if (co_ppm <= 15.0) {
            digitalWrite(LED_GREEN_PIN, LOW);
            digitalWrite(LED_RED_PIN, HIGH);
            tone(BUZZER_PIN, 1000);
            display.println(F("STATUS: SMOKER DETECTED"));

        } else {
            digitalWrite(LED_GREEN_PIN, LOW);
            digitalWrite(LED_RED_PIN, HIGH);
            tone(BUZZER_PIN, 2000);
            delay(100);
            digitalWrite(LED_RED_PIN, LOW);
            noTone(BUZZER_PIN);
            delay(100);
            display.println(F("DANGER: HEAVY SMOKER!!"));
        }

        display.display();
        delay(200);
    }
}