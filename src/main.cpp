#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "dashboard.h"

// --- WiFi ---
// #define WIFI_SSID "UPB-Guest"
// #define WIFI_PASSWORD ""

#define WIFI_SSID "DIGI-73bC"
#define WIFI_PASSWORD "A3tNcGdsa5"

// --- OLED SPI ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_MOSI 18
#define OLED_CLK  19
#define OLED_DC   4
#define OLED_CS   5
#define OLED_RESET 2

// --- MAX30102 I2C ---
#define I2C_SDA 6
#define I2C_SCL 7

// --- Periferice ---
#define BUZZER_PIN 10
#define FLUID_SENSOR 11
#define ECG_PIN 0

#define BUFFER_LENGTH 100

// --- Obiecte ---
Adafruit_SSD1306 display(
    SCREEN_WIDTH, SCREEN_HEIGHT,
    OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS
);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
MAX30105 particleSensor;

// --- Variabile MAX30102 ---
uint32_t irBuffer[BUFFER_LENGTH];
uint32_t redBuffer[BUFFER_LENGTH];
int32_t spo2 = 0;
int8_t validSPO2 = 0;
int32_t heartRate = 0;
int8_t validHeartRate = 0;
bool maxConnected = false;
long lastIR = 0;

// --- Variabile senzori ---
bool fluidDetected = true;
int ecgValue = 0;

// --- Timer ISR ---
hw_timer_t *timer = NULL;
volatile bool readSensorsFlag = false;

// --- Timere non-blocking ---
unsigned long lastBroadcast = 0;
unsigned long lastMAXUpdate = 0;
unsigned long lastOLEDUpdate = 0;

// ISR - se executa la fiecare 10ms
// Rutina de tratare a intreruperii
void IRAM_ATTR onTimer() {
    readSensorsFlag = true;
}

// Functii
void buzzerOn(int frequency) {
    // atasam PWM + duty cycle mic => volum mic sonor
    ledcAttach(BUZZER_PIN, frequency, 8);
    ledcWrite(BUZZER_PIN, 30);
}

void buzzerOff() {
    ledcWrite(BUZZER_PIN, 0);
    ledcDetach(BUZZER_PIN);
}

// Creează un obiect JSON 
// cu toate datele și îl trimite prin WebSocket către browser.
// validHeartRate ? heartRate : 0 înseamnă — dacă valoarea e validă trimite-o, altfel trimite 0.
void broadcastData() {
    JsonDocument doc;
    doc["ecg"] = ecgValue;
    doc["fluid"] = fluidDetected;
    doc["hr"] = validHeartRate ? heartRate : 0;
    doc["spo2"] = validSPO2 ? spo2 : 0;
    String output;
    serializeJson(doc, output);
    ws.textAll(output);
}

// display pe oled
void drawScreen() {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    display.setCursor(10, 0);
    display.println("VITALGUARD LIVE");
    display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    display.setCursor(0, 12);
    display.print("IP: ");
    display.println(WiFi.localIP());

    display.setCursor(0, 24);
    display.print("Fluid: ");
    display.println(fluidDetected ? "OK" : "LOW!");

    display.setCursor(0, 34);
    display.print("HR: ");
    display.print(validHeartRate ? heartRate : 0);
    display.println(" BPM");

    display.setCursor(0, 44);
    display.print("SpO2: ");
    display.print(validSPO2 ? spo2 : 0);
    display.println(" %");

    display.setCursor(0, 54);
    display.print("ECG: ");
    display.println(ecgValue);

    display.display();
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("Client #%u conectat\n", client->id());
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("Client #%u deconectat\n", client->id());
    }
}

void readMAXSamples(int start, int end) {
    for (int i = start; i < end; i++) {
        while (!particleSensor.available())
            particleSensor.check();
        redBuffer[i] = particleSensor.getRed();
        irBuffer[i] = particleSensor.getIR();
        particleSensor.nextSample();
    }
}

// intiializare modul  cu I2C
// calibrare initiala cu 100 de esantioane 
void initMAX30102() {
    Wire.begin(I2C_SDA, I2C_SCL);

    if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
        Serial.println("MAX30102 negasit!");
        maxConnected = false;
        return;
    }

    Serial.println("MAX30102 gasit!");
    maxConnected = true;

    particleSensor.setup(0x3F, 4, 2, 100, 411, 16384);
    particleSensor.setPulseAmplitudeRed(0x3F);
    particleSensor.setPulseAmplitudeIR(0x3F);
    particleSensor.setPulseAmplitudeGreen(0);

    Serial.println("Calibrare MAX30102...");
    readMAXSamples(0, BUFFER_LENGTH); // colectam 100 de esantioane

    // primul ciclu:
    maxim_heart_rate_and_oxygen_saturation(
        irBuffer, BUFFER_LENGTH, redBuffer,
        &spo2, &validSPO2, &heartRate, &validHeartRate
    );

    Serial.println("Calibrare finalizata!");
}

// actualizare cu sliding window
// in loc sa colectam 100 de esantionae de fiecare data -> adaugam 25 esantionae noi si pastram 75
void updateMAX30102() {
    if (!maxConnected) return;

    for (int i = 25; i < BUFFER_LENGTH; i++) {
        redBuffer[i - 25] = redBuffer[i];
        irBuffer[i - 25] = irBuffer[i];
    }

    readMAXSamples(75, BUFFER_LENGTH);

    maxim_heart_rate_and_oxygen_saturation(
        irBuffer, BUFFER_LENGTH, redBuffer,
        &spo2, &validSPO2, &heartRate, &validHeartRate
    );

    lastIR = irBuffer[BUFFER_LENGTH - 1];
}

void setup() {
    // 1. Initializeaza pinii
    // 2. Porneste OLED
    // 3. Conecteaza WiFi - asteapta pana se conecteaza
    // 4. Initializeaza MAX30102
    // 5. Configureaza timerul ISR la 10ms

    Serial.begin(115200);
    delay(3000);
    Serial.println("\n--- Initializing System ---");

    pinMode(FLUID_SENSOR, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    // OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC)) {
        Serial.println("OLED ERROR");
    } else {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("Connecting WiFi...");
        display.display();
        Serial.println("OLED OK");
    }

    // WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");
    Serial.println(WiFi.localIP());

    // MAX30102
    initMAX30102();

    // Timer hardware ISR la 10ms
    timer = timerBegin(1000000);
    timerAttachInterrupt(timer, &onTimer);
    timerAlarm(timer, 10000, true, 0);
    Serial.println("Timer ISR pornit!");

    // Web server
    ws.onEvent(onEvent);
    server.addHandler(&ws);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html);
    });
    server.begin();
    Serial.println("Server pornit!");
}

void loop() {
    ws.cleanupClients(); // curata clientii deconectati

    // ISR - cel mai prioritar - fluid + ECG + buzzer la 10ms
    if (readSensorsFlag) {
        readSensorsFlag = false;

        fluidDetected = (digitalRead(FLUID_SENSOR) == HIGH);
        ecgValue = analogRead(ECG_PIN);

        if (!fluidDetected) {
            buzzerOn(2000);
        } else {
            buzzerOff();
        }
    }

    // Broadcast la 10ms
    if (millis() - lastBroadcast > 20) {
        lastBroadcast = millis();
        broadcastData(); // trimitem date browser-ului
    }

    // OLED la 200ms
    if (millis() - lastOLEDUpdate > 200) {
        lastOLEDUpdate = millis();
        drawScreen();
    }

    // MAX30102 la 1000ms
    if (millis() - lastMAXUpdate > 5000) {
        lastMAXUpdate = millis();
        updateMAX30102();
    }
}