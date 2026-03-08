/*
 * VERTEX UWB - Anchor with WiFi + MQTT + Piggyback Sensor + Panic
 * 
 * Publishes UWB distance data, gas sensor data, and panic status
 * (all received via AT+RDATA) to MQTT broker
 * 
 * Hardware: MakerFabs ESP32-S3 UWB Pro (DW3000 + STM32 AT)
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include "../../config.h"

// Anchor Configuration - CHANGE THIS SECTION WHEN PROGRAMMING ANCHORS
#define ANCHOR_ID 0
#define ANCHOR_X 0
#define ANCHOR_Y 0

// UWB Settings
#define UWB_TAG_COUNT 64
// =======================================================

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

// WiFi and MQTT clients
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// State variables
String response = "";
bool wifiConnected = false;
bool mqttConnected = false;
unsigned long lastReconnectAttempt = 0;

// Cached sensor data from tag (received via AT+RDATA)
struct SensorData {
    float temperature;
    float humidity;
    float pressure;
    float gas;
    bool panic;
    bool valid;
    unsigned long lastUpdate;
};

// Support up to 8 tags with sensor data
SensorData tagSensors[8];

void logoshow() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("VERTEX Anchor+MQTT"));
    
    display.setCursor(0, 16);
    display.setTextSize(2);
    display.print("A");
    display.println(ANCHOR_ID);
    
    display.setTextSize(1);
    display.setCursor(0, 40);
    display.print("Pos: ");
    display.print(ANCHOR_X);
    display.print(",");
    display.println(ANCHOR_Y);
    
    display.display();
    delay(2000);
}

void updateDisplay(String status, String data = "") {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    display.setCursor(0, 0);
    display.print("A");
    display.print(ANCHOR_ID);
    display.print(" WiFi:");
    display.print(wifiConnected ? "OK" : "--");
    display.print(" MQ:");
    display.println(mqttConnected ? "OK" : "--");
    
    display.setCursor(0, 16);
    display.println(status);
    
    if (data.length() > 0) {
        display.setCursor(0, 32);
        if (data.length() > 21) {
            display.println(data.substring(0, 21));
        } else {
            display.println(data);
        }
    }
    
    // Show sensor data and panic status
    display.setCursor(0, 48);
    if (tagSensors[0].valid) {
        if (tagSensors[0].panic) {
            display.println("!! TAG 0 PANIC !!");
        } else {
            display.print("Gas T:");
            display.print(tagSensors[0].temperature, 1);
            display.print(" G:");
            display.print(tagSensors[0].gas, 0);
        }
    }
    
    display.display();
}

void setupWiFi() {
    SERIAL_LOG.print("Connecting to WiFi: ");
    SERIAL_LOG.println(WIFI_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        SERIAL_LOG.print(".");
        attempts++;
        updateDisplay("WiFi connecting...");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        SERIAL_LOG.println();
        SERIAL_LOG.print("WiFi connected! IP: ");
        SERIAL_LOG.println(WiFi.localIP());
    } else {
        SERIAL_LOG.println();
        SERIAL_LOG.println("WiFi connection failed!");
    }
}

void setupMQTT() {
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    SERIAL_LOG.print("MQTT server: ");
    SERIAL_LOG.print(MQTT_SERVER);
    SERIAL_LOG.print(":");
    SERIAL_LOG.println(MQTT_PORT);
}

void reconnectMQTT() {
    if (!wifiConnected) return;
    
    if (millis() - lastReconnectAttempt < 5000) return;
    lastReconnectAttempt = millis();
    
    SERIAL_LOG.print("Connecting to MQTT...");
    
    String clientId = "vertex-anchor-";
    clientId += ANCHOR_ID;
    
    bool connected;
    if (strlen(MQTT_USER) > 0) {
        connected = mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD);
    } else {
        connected = mqttClient.connect(clientId.c_str());
    }
    
    if (connected) {
        mqttConnected = true;
        SERIAL_LOG.println("connected!");
        
        String topic = "uwb/anchor/";
        topic += ANCHOR_ID;
        topic += "/status";
        mqttClient.publish(topic.c_str(), "online", true);
    } else {
        mqttConnected = false;
        SERIAL_LOG.print("failed, rc=");
        SERIAL_LOG.println(mqttClient.state());
    }
}

void publishDistance(int tagId, float distance, int rssi) {
    if (!mqttConnected) return;
    
    JsonDocument doc;
    doc["tagId"] = tagId;
    doc["distance"] = distance;
    doc["rssi"] = rssi;
    doc["anchorId"] = ANCHOR_ID;
    doc["anchorX"] = ANCHOR_X;
    doc["anchorY"] = ANCHOR_Y;
    doc["timestamp"] = millis();
    
    // Include sensor data if we have recent data for this tag (within 15 seconds)
    if (tagId < 8 && tagSensors[tagId].valid && 
        (millis() - tagSensors[tagId].lastUpdate) < 15000) {
        doc["temperature"] = tagSensors[tagId].temperature;
        doc["humidity"] = tagSensors[tagId].humidity;
        doc["pressure"] = tagSensors[tagId].pressure;
        doc["gas"] = tagSensors[tagId].gas;
        doc["panic"] = tagSensors[tagId].panic;
    }
    
    String payload;
    serializeJson(doc, payload);
    
    String topic = "uwb/anchor/";
    topic += ANCHOR_ID;
    
    if (mqttClient.publish(topic.c_str(), payload.c_str())) {
        SERIAL_LOG.print("[MQTT] Published: ");
        SERIAL_LOG.println(payload);
    } else {
        SERIAL_LOG.println("[MQTT] Publish failed!");
    }
}

// Parse sensor payload: "T:25.3,H:45.2,P:1013.2,G:52.1,E:0"
bool parseSensorPayload(String payload, int tagId) {
    if (tagId < 0 || tagId >= 8) return false;
    
    float temp = 0, hum = 0, pres = 0, gas = 0;
    bool panic = false;
    bool gotT = false, gotH = false, gotP = false, gotG = false;
    
    int idx = 0;
    while (idx < payload.length()) {
        char key = payload.charAt(idx);
        if (idx + 1 < payload.length() && payload.charAt(idx + 1) == ':') {
            int valStart = idx + 2;
            int valEnd = payload.indexOf(',', valStart);
            if (valEnd < 0) valEnd = payload.length();
            
            String valStr = payload.substring(valStart, valEnd);
            
            switch (key) {
                case 'T': temp = valStr.toFloat(); gotT = true; break;
                case 'H': hum = valStr.toFloat(); gotH = true; break;
                case 'P': pres = valStr.toFloat(); gotP = true; break;
                case 'G': gas = valStr.toFloat(); gotG = true; break;
                case 'E': panic = (valStr.toInt() == 1); break;
            }
            
            idx = valEnd + 1;
        } else {
            idx++;
        }
    }
    
    if (gotT && gotH && gotP && gotG) {
        tagSensors[tagId].temperature = temp;
        tagSensors[tagId].humidity = hum;
        tagSensors[tagId].pressure = pres;
        tagSensors[tagId].gas = gas;
        tagSensors[tagId].panic = panic;
        tagSensors[tagId].valid = true;
        tagSensors[tagId].lastUpdate = millis();
        
        SERIAL_LOG.print("[SENSOR] Tag ");
        SERIAL_LOG.print(tagId);
        SERIAL_LOG.print(" -> T:");
        SERIAL_LOG.print(temp, 1);
        SERIAL_LOG.print(" H:");
        SERIAL_LOG.print(hum, 1);
        SERIAL_LOG.print(" P:");
        SERIAL_LOG.print(pres, 1);
        SERIAL_LOG.print(" G:");
        SERIAL_LOG.print(gas, 1);
        SERIAL_LOG.print(" PANIC:");
        SERIAL_LOG.println(panic ? "YES" : "no");
        return true;
    }
    
    SERIAL_LOG.print("[SENSOR] Parse failed: ");
    SERIAL_LOG.println(payload);
    return false;
}

// Parse AT+RDATA line
// Format: AT+RDATA=Tag,0,1799048,35,T:25.3,H:45.2,P:1013.2,G:52.1,E:0
void parseRDATA(String data) {
    if (!data.startsWith("AT+RDATA=")) return;
    
    String params = data.substring(9);
    
    int commaPos[4];
    int found = 0;
    for (int i = 0; i < params.length() && found < 4; i++) {
        if (params.charAt(i) == ',') {
            commaPos[found] = i;
            found++;
        }
    }
    
    if (found < 4) {
        SERIAL_LOG.print("[RDATA] Not enough fields: ");
        SERIAL_LOG.println(data);
        return;
    }
    
    int tagId = params.substring(commaPos[0] + 1, commaPos[1]).toInt();
    String payload = params.substring(commaPos[3] + 1);
    
    SERIAL_LOG.print("[RDATA] Tag ");
    SERIAL_LOG.print(tagId);
    SERIAL_LOG.print(" data: ");
    SERIAL_LOG.println(payload);
    
    parseSensorPayload(payload, tagId);
}

void parseAndPublish(String data) {
    if (!data.startsWith("AT+RANGE=")) return;
    
    int tidIdx = data.indexOf("tid:");
    if (tidIdx < 0) return;
    int tidEnd = data.indexOf(",", tidIdx);
    int tagId = data.substring(tidIdx + 4, tidEnd).toInt();
    
    int rangeIdx = data.indexOf("range:(");
    if (rangeIdx < 0) return;
    int rangeStart = rangeIdx + 7;
    int rangeEnd = data.indexOf(")", rangeStart);
    String rangeStr = data.substring(rangeStart, rangeEnd);
    
    int ancidIdx = data.indexOf("ancid:(");
    if (ancidIdx < 0) return;
    int ancidStart = ancidIdx + 7;
    int ancidEnd = data.indexOf(")", ancidStart);
    String ancidStr = data.substring(ancidStart, ancidEnd);
    
    int rangeValues[8] = {0};
    int ancidValues[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
    
    int idx = 0;
    int startPos = 0;
    for (int i = 0; i <= rangeStr.length() && idx < 8; i++) {
        if (i == rangeStr.length() || rangeStr[i] == ',') {
            rangeValues[idx] = rangeStr.substring(startPos, i).toInt();
            startPos = i + 1;
            idx++;
        }
    }
    
    idx = 0;
    startPos = 0;
    for (int i = 0; i <= ancidStr.length() && idx < 8; i++) {
        if (i == ancidStr.length() || ancidStr[i] == ',') {
            ancidValues[idx] = ancidStr.substring(startPos, i).toInt();
            startPos = i + 1;
            idx++;
        }
    }
    
    int distanceCm = -1;
    for (int i = 0; i < 8; i++) {
        if (ancidValues[i] == ANCHOR_ID) {
            distanceCm = rangeValues[i];
            break;
        }
    }
    
    if (distanceCm > 0) {
        float distanceM = distanceCm / 100.0;
        publishDistance(tagId, distanceM, 0);
        
        String displayData = "T";
        displayData += tagId;
        displayData += ": ";
        displayData += String(distanceM, 2);
        displayData += "m";
        updateDisplay("Ranging OK", displayData);
    }
}

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
    temp += ANCHOR_ID;
    temp += ",1";  // Role: 1 = Anchor
    temp += ",1";  // Speed: 1 = 6.8M
    temp += ",1";  // Range filter: on
    return temp;
}

String cap_cmd() {
    String temp = "AT+SETCAP=";
    temp += UWB_TAG_COUNT;
    temp += ",10";
    temp += ",1";
    return temp;
}

void setup() {
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(RESET_PIN, HIGH);
    
    SERIAL_LOG.begin(115200);
    delay(2000);
    
    SERIAL_LOG.println(F("\n\n================================================"));
    SERIAL_LOG.println(F("  VERTEX UWB - Anchor + MQTT + Piggyback"));
    SERIAL_LOG.println(F("================================================\n"));
    
    // Initialize sensor data cache
    for (int i = 0; i < 8; i++) {
        tagSensors[i].valid = false;
        tagSensors[i].panic = false;
        tagSensors[i].lastUpdate = 0;
    }
    
    SERIAL_AT.begin(115200, SERIAL_8N1, IO_RXD2, IO_TXD2);
    SERIAL_AT.println("AT");
    
    Wire.begin(I2C_SDA, I2C_SCL);
    delay(1000);
    
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        SERIAL_LOG.println(F("SSD1306 OLED failed!"));
    }
    display.clearDisplay();
    logoshow();
    
    SERIAL_LOG.println(F("Configuring UWB module..."));
    updateDisplay("UWB Config...");
    
    sendData("AT?", 2000, true);
    sendData("AT+RESTORE", 5000, true);
    sendData(config_cmd(), 2000, true);
    sendData(cap_cmd(), 2000, true);
    sendData("AT+SETRPT=1", 2000, true);
    sendData("AT+SAVE", 2000, true);
    sendData("AT+RESTART", 2000, true);
    
    SERIAL_LOG.println(F("UWB configured as Anchor!"));
    
    setupWiFi();
    setupMQTT();
    
    SERIAL_LOG.println(F("\n================================================"));
    SERIAL_LOG.println(F("  Anchor Ready - MQTT + Piggyback + Panic"));
    SERIAL_LOG.println(F("================================================\n"));
    
    updateDisplay("Ready", "Waiting for tags...");
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        wifiConnected = false;
        mqttConnected = false;
        static unsigned long lastWifiAttempt = 0;
        if (millis() - lastWifiAttempt > 10000) {
            lastWifiAttempt = millis();
            WiFi.reconnect();
        }
    } else {
        wifiConnected = true;
    }
    
    if (wifiConnected && !mqttClient.connected()) {
        mqttConnected = false;
        reconnectMQTT();
    }
    
    if (mqttConnected) {
        mqttClient.loop();
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
                SERIAL_LOG.print("[UWB] ");
                SERIAL_LOG.println(response);
                
                if (response.startsWith("AT+RANGE")) {
                    parseAndPublish(response);
                } else if (response.startsWith("AT+RDATA")) {
                    parseRDATA(response);
                }
            }
            response = "";
        } else {
            response += c;
        }
    }
}