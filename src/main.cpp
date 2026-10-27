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

// --- CO READ FUNCTION ---
float readCOPpm() {
    int rawAnalog = analogRead(SENSOR_PIN);
    float voltage = (rawAnalog / 4095.0) * 3.3;
    float ppm = (voltage / 3.3) * 100.0; // คำนวณตาม Curve ของเซนเซอร์ MQ
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
    float co_ppm = readCOPpm();

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F(" ScoutAntiDRUG eCO"));
    display.println(F("---------------------"));

    // แสดงค่า PPM ตัวโต
    display.setTextSize(2);
    display.setCursor(10, 20);
    display.print(co_ppm, 1);
    display.println(F(" PPM"));

    display.setTextSize(1);
    display.setCursor(0, 48);

    // --- INTERPRETATION LOGIC (ตามตาราง 4 ระดับ) ---
    if (co_ppm <= 6.0) {
        // 0 - 6 ppm: ปกติ (Non-Smoker)
        digitalWrite(LED_GREEN_PIN, HIGH);
        digitalWrite(LED_RED_PIN, LOW);
        noTone(BUZZER_PIN);
        display.println(F("STATUS: CLEAN (NORMAL)"));

    } else if (co_ppm <= 10.0) {
        // 7 - 10 ppm: เสี่ยง / สูบประปราย (Light Smoker)
        digitalWrite(LED_GREEN_PIN, HIGH);
        digitalWrite(LED_RED_PIN, HIGH); // ติดสองสีเป็นเตือนเหลือง
        noTone(BUZZER_PIN);
        display.println(F("STATUS: LIGHT SMOKER"));

    } else if (co_ppm <= 15.0) {
        // 11 - 15 ppm: สูบจริงชัดเจน (Smoker)
        digitalWrite(LED_GREEN_PIN, LOW);
        digitalWrite(LED_RED_PIN, HIGH);
        tone(BUZZER_PIN, 1000); // ส่งเสียงเตือนยาว
        display.println(F("STATUS: SMOKER DETECTED"));

    } else {
        // 16+ ppm: สูบจัด / เพิ่งสูบมาทันที (Heavy Smoker) - คาหนังคาเขา!
        digitalWrite(LED_GREEN_PIN, LOW);
        
        // ไฟแดงกะพริบ + Buzzer จังหวะถี่กระชั้น (Alert ขั้นสูงสุด)
        digitalWrite(LED_RED_PIN, HIGH);
        tone(BUZZER_PIN, 2000);
        delay(150);
        digitalWrite(LED_RED_PIN, LOW);
        noTone(BUZZER_PIN);
        delay(150);

        display.println(F("DANGER: HEAVY SMOKER!!"));
    }

    display.display();
    delay(300);
}