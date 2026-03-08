/*
 * VERTEX UWB - Tag 0 (Piggyback Mode + Panic Button + Passive Buzzer)
 * 
 * Sends BME680 sensor data + panic status over UWB using AT+DATA
 * Panic button on GPIO 9 (active LOW, external pull-up to 3V3)
 *   - Hold 1 second  -> activate panic
 *   - Hold 3 seconds -> deactivate panic
 * Passive Buzzer on GPIO 5 (LEDC PWM)
 *   - Currently: constant 4kHz buzz for hardware verification
 *   - Later: triggers on hazardous gas (IAQ >= 200) or panic
 * 
 * Hardware: MakerFabs ESP32-S3 UWB Pro (DW3000 + STM32 AT)
 * Sensor:  BME680 on I2C (SDA=6, SCL=7)
 * Button:  GPIO 9 (J1 pin 9, pull-up to 3V3)
 * Buzzer:  GPIO 5 (J1 pin 5, LEDC PWM) - PASSIVE buzzer
 */

#include <Adafruit_BME680.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

// ============== USER CONFIG ==============
#define UWB_INDEX 0          // Tag ID (0-63)
#define UWB_TAG_COUNT 64     // Max tags in system
#define SENSOR_INTERVAL 5000 // Send sensor data every 5 seconds
// =========================================

// Panic button
#define BUTTON_PIN 9         // GPIO 9, active LOW (external pull-up)
#define PANIC_ACTIVATE_MS  1000  // Hold 1s to activate
#define PANIC_DEACTIVATE_MS 3000 // Hold 3s to deactivate

bool panicActive = false;
bool buttonPressed = false;
unsigned long buttonPressStart = 0;
bool actionTaken = false;

// Buzzer (LEDC PWM)
#define BUZZER_PIN      5       // IO5 — J1 pin 5
#define BUZZER_CHANNEL  0       // LEDC channel 0
#define BUZZER_FREQ     4000    // 4 kHz (gas alert tone)
#define BUZZER_RES      8       // 8-bit resolution (0–255)
#define BUZZER_FREQ_HIGH 2500   // Wee-woo high tone (Hz)
#define BUZZER_FREQ_LOW  1500   // Wee-woo low tone (Hz)
#define BUZZER_TOGGLE_MS 500    // Alternate every 500ms

bool buzzerActive = false;
bool buzzerInPanicMode = false;   // true = wee-woo, false = steady
unsigned long lastBuzzerToggle = 0;
bool buzzerHigh = true;



// Gas alert threshold (matches dashboard IAQ >= 200)
#define GAS_ALERT_THRESHOLD_KOHM 30.0  // Below 30 kOhm = IAQ ~200+ (hazardous)

bool gasAlert = false;

// BME680 on second I2C bus (pins 6, 7)
TwoWire I2C_BME = TwoWire(1);
Adafruit_BME680 bme(&I2C_BME);
bool bmeReady = false;

// Pin definitions for ESP32-S3 UWB board
#define RESET_PIN 16
#define IO_RXD2 18
#define IO_TXD2 17
#define I2C_SDA 39
#define I2C_SCL 38

// Serial aliases
#define SERIAL_LOG Serial
HardwareSerial SERIAL_AT(2);

// OLED Display
Adafruit_SSD1306 display(128, 64, &Wire, -1);

String response = "";
unsigned long lastSensorSend = 0;
float lastGasResistance = 0;

// ==================== BUZZER FUNCTIONS ====================

void buzzerOn(bool panicMode) {
    buzzerActive = true;
    buzzerInPanicMode = panicMode;
    
    if (panicMode) {
        // Wee-woo pattern — start with high tone
        buzzerHigh = true;
        lastBuzzerToggle = millis();
        ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ_HIGH, BUZZER_RES);
        ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
        ledcWrite(BUZZER_CHANNEL, 128);
        SERIAL_LOG.println("[BUZZER] ON - emergency wee-woo");
    } else {
        // Steady 4kHz tone for gas alert
        ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, BUZZER_RES);
        ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
        ledcWrite(BUZZER_CHANNEL, 128);
        SERIAL_LOG.println("[BUZZER] ON - gas alert steady tone");
    }
}

void buzzerOff() {
    if (buzzerActive) {
        buzzerActive = false;
        buzzerInPanicMode = false;
        ledcWrite(BUZZER_CHANNEL, 0);
        SERIAL_LOG.println("[BUZZER] OFF");
    }
}

// Call this in loop() — handles the wee-woo alternation
void updateBuzzer() {
    if (!buzzerActive || !buzzerInPanicMode) return;

    if (millis() - lastBuzzerToggle >= BUZZER_TOGGLE_MS) {
        lastBuzzerToggle = millis();
        buzzerHigh = !buzzerHigh;
        uint32_t freq = buzzerHigh ? BUZZER_FREQ_HIGH : BUZZER_FREQ_LOW;
        ledcSetup(BUZZER_CHANNEL, freq, BUZZER_RES);
        ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
        ledcWrite(BUZZER_CHANNEL, 128);
    }
}

void checkBuzzerState() {
    if (panicActive) {
        // Emergency takes priority — wee-woo
        if (!buzzerActive || !buzzerInPanicMode) {
            buzzerOff();
            buzzerOn(true);
        }
    } else if (gasAlert) {
        // Gas alert — steady tone
        if (!buzzerActive || buzzerInPanicMode) {
            buzzerOff();
            buzzerOn(false);
        }
    } else {
        // All clear
        buzzerOff();
    }
}



// ==================== DISPLAY FUNCTIONS ====================

void logoshow() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("VERTEX Tag Piggyback"));
    
    display.setCursor(0, 16);
    display.setTextSize(2);
    display.print("T");
    display.println(UWB_INDEX);
    
    display.setTextSize(1);
    display.setCursor(0, 40);
    display.println("BME680+Panic+Buzzer");
    display.println("No WiFi needed");
    
    display.display();
    delay(2000);
}

void updateDisplay(String line1, String line2 = "", String line3 = "") {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    display.setCursor(0, 0);
    display.print("T");
    display.print(UWB_INDEX);
    if (panicActive) {
        display.println(" !! PANIC !!");
    } else if (gasAlert) {
        display.println(" !! GAS ALERT !!");
    } else {
        display.println(" PIGGYBACK");
    }
    
    display.setCursor(0, 16);
    display.println(line1);
    
    if (line2.length() > 0) {
        display.setCursor(0, 32);
        display.println(line2);
    }
    if (line3.length() > 0) {
        display.setCursor(0, 48);
        display.println(line3);
    }
    
    display.display();
}

// ==================== UWB FUNCTIONS ====================

String sendData(String command, const int timeout, boolean debug) {
    String resp = "";
    
    SERIAL_LOG.print("[TX] ");
    SERIAL_LOG.println(command);
    SERIAL_AT.println(command);
    
    unsigned long startTime = millis();
    while ((startTime + timeout) > millis()) {
        while (SERIAL_AT.available()) {
            char c = SERIAL_AT.read();
            resp += c;
        }
    }
    
    if (debug) {
        SERIAL_LOG.print("[RX] ");
        SERIAL_LOG.println(resp);
    }
    
    return resp;
}

String config_cmd() {
    String temp = "AT+SETCFG=";
    temp += UWB_INDEX;
    temp += ",0";
    temp += ",1";
    temp += ",1";
    return temp;
}

String cap_cmd() {
    String temp = "AT+SETCAP=";
    temp += UWB_TAG_COUNT;
    temp += ",10";
    temp += ",1";
    return temp;
}

// ==================== SENSOR FUNCTIONS ====================

void setupBME680() {
    I2C_BME.begin(6, 7);
    if (!bme.begin(0x76)) {
        if (!bme.begin(0x77)) {
            SERIAL_LOG.println("BME680 not found!");
            updateDisplay("BME680 FAILED!", "Check wiring");
            return;
        }
    }
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150);
    bmeReady = true;
    SERIAL_LOG.println("BME680 ready!");
}

void sendSensorData() {
    if (!bmeReady) return;
    if (!bme.performReading()) {
        SERIAL_LOG.println("[SENSOR] BME680 read failed!");
        return;
    }
    
    float gasKohm = bme.gas_resistance / 1000.0;
    lastGasResistance = gasKohm;

    // Check gas alert: below threshold = hazardous (IAQ >= 200)
    gasAlert = (gasKohm < GAS_ALERT_THRESHOLD_KOHM);
    checkBuzzerState();

    // Format: T:25.3,H:45.2,P:1013.2,G:52.1,E:0
    String payload = "T:";
    payload += String(bme.temperature, 1);
    payload += ",H:";
    payload += String(bme.humidity, 1);
    payload += ",P:";
    payload += String(bme.pressure / 100.0, 1);
    payload += ",G:";
    payload += String(gasKohm, 1);
    payload += ",E:";
    payload += panicActive ? "1" : "0";
    
    String cmd = "AT+DATA=";
    cmd += payload.length();
    cmd += ",";
    cmd += payload;
    
    SERIAL_AT.println(cmd);
    
    SERIAL_LOG.print("[SENSOR] Sent via UWB: ");
    SERIAL_LOG.println(payload);
    SERIAL_LOG.print("[SENSOR] Gas: ");
    SERIAL_LOG.print(gasKohm, 1);
    SERIAL_LOG.print(" kOhm | Alert: ");
    SERIAL_LOG.println(gasAlert ? "YES" : "no");
    
    // Update display
    String tempStr = "T:" + String(bme.temperature, 1) + "C";
    String humStr = "H:" + String(bme.humidity, 1) + "%";
    String gasStr = "G:" + String(gasKohm, 1) + "kOhm";
    if (panicActive) {
        gasStr = "!! PANIC ACTIVE !!";
    } else if (gasAlert) {
        gasStr = "!! GAS HAZARD !!";
    }
    updateDisplay(tempStr, humStr, gasStr);
}

void sendPanicUpdate() {
    if (!bmeReady) {
        String payload = "T:0,H:0,P:0,G:0,E:";
        payload += panicActive ? "1" : "0";
        
        String cmd = "AT+DATA=";
        cmd += payload.length();
        cmd += ",";
        cmd += payload;
        SERIAL_AT.println(cmd);
        return;
    }
    
    sendSensorData();
}

// ==================== BUTTON HANDLER ====================

void handleButton() {
    bool currentlyPressed = (digitalRead(BUTTON_PIN) == LOW);
    
    if (currentlyPressed && !buttonPressed) {
        buttonPressed = true;
        buttonPressStart = millis();
        actionTaken = false;
        SERIAL_LOG.println("[BTN] Button pressed");
    } 
    else if (currentlyPressed && buttonPressed && !actionTaken) {
        unsigned long holdTime = millis() - buttonPressStart;
        
        if (!panicActive && holdTime >= PANIC_ACTIVATE_MS) {
            panicActive = true;
            actionTaken = true;
            SERIAL_LOG.println("[BTN] !! PANIC ACTIVATED !!");
            checkBuzzerState();
            sendPanicUpdate();
            
            display.clearDisplay();
            display.setTextSize(2);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(10, 20);
            display.println("!! PANIC !!");
            display.setTextSize(1);
            display.setCursor(10, 50);
            display.println("Hold 3s to cancel");
            display.display();
        } 
        else if (panicActive && holdTime >= PANIC_DEACTIVATE_MS) {
            panicActive = false;
            actionTaken = true;
            SERIAL_LOG.println("[BTN] Panic DEACTIVATED");
            checkBuzzerState();
            sendPanicUpdate();
            
            display.clearDisplay();
            display.setTextSize(2);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(10, 20);
            display.println("PANIC OFF");
            display.setTextSize(1);
            display.setCursor(10, 50);
            display.println("Resuming normal...");
            display.display();
            delay(1000);
        }
    } 
    else if (!currentlyPressed && buttonPressed) {
        buttonPressed = false;
        SERIAL_LOG.print("[BTN] Released after ");
        SERIAL_LOG.print(millis() - buttonPressStart);
        SERIAL_LOG.println("ms");
    }
}

// ==================== SETUP & LOOP ====================

void setup() {

    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(RESET_PIN, HIGH);
    
    // Panic button
    pinMode(BUTTON_PIN, INPUT);

    // Setup buzzer PWM — starts silent, activates on alert/panic
    ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, BUZZER_RES);
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
    ledcWrite(BUZZER_CHANNEL, 0);  // Start silent
    
    SERIAL_LOG.begin(115200);
    delay(2000);
    
    SERIAL_LOG.println(F("\n\n=========================================="));
    SERIAL_LOG.println(F("  VERTEX UWB - Tag 0 (Piggyback+Panic+Buzzer)"));
    SERIAL_LOG.println(F("  BME680 + Panic GPIO9 + Passive Buzzer GPIO5"));
    SERIAL_LOG.println(F("==========================================\n"));
    
    SERIAL_AT.begin(115200, SERIAL_8N1, IO_RXD2, IO_TXD2);
    SERIAL_AT.println("AT");
    
    Wire.begin(I2C_SDA, I2C_SCL);
    delay(1000);
    
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        SERIAL_LOG.println(F("SSD1306 OLED failed!"));
    }
    display.clearDisplay();
    logoshow();
    
    setupBME680();
    
    SERIAL_LOG.println(F("Configuring UWB module as Tag 0..."));
    
    sendData("AT?", 2000, true);
    sendData("AT+RESTORE", 5000, true);
    sendData(config_cmd(), 2000, true);
    sendData(cap_cmd(), 2000, true);
    sendData("AT+SETRPT=1", 2000, true);
    sendData("AT+SAVE", 2000, true);
    sendData("AT+RESTART", 2000, true);
    
    SERIAL_LOG.println(F("\n=========================================="));
    SERIAL_LOG.println(F("  Tag 0 Ready - Piggyback+Panic+Buzzer"));
    SERIAL_LOG.println(F("  Btn: 1s=SOS, 3s=CANCEL"));
    SERIAL_LOG.println(F("  Buzzer: constant 4kHz (testing)"));
    SERIAL_LOG.println(F("==========================================\n"));
    
    updateDisplay("Ready", "Waiting to range...", "Btn:1s=SOS Buzz:GPIO5");
}

void loop() {
    handleButton();
    updateBuzzer();
    
    if (millis() - lastSensorSend >= SENSOR_INTERVAL) {
        lastSensorSend = millis();
        sendSensorData();
    }
    
    while (SERIAL_LOG.available() > 0) {
        SERIAL_AT.write(SERIAL_LOG.read());
        yield();
    }
    
    while (SERIAL_AT.available() > 0) {
        char c = SERIAL_AT.read();
        
        if (c == '\r') {
            continue;
        } else if (c == '\n') {
            if (response.length() > 0) {
                SERIAL_LOG.println(response);
            }
            response = "";
        } else {
            response += c;
        }
    }
}