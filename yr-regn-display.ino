#define LILYGO_T5_V213
#include <boards.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include "config.h" // Environment variables
#include <esp_sleep.h>
#include <sys/time.h>
#include <math.h>

// Constants
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* yrApiUrl = "https://www.yr.no/api/v0/locations/" YR_LOCATION "/forecast/now";
const int BUTTON_PIN = 39; // LilyGo T5 integrated button pin
const int UPDATE_INTERVAL = 5 * 60 * 1000000;  // 5 minutes in microseconds

// Global variables
float precipitationData[18]; // Array to store 90 minutes of precipitation data (5-minute intervals)
String responseTime; // String to store the time from the HTTP response
String createdTime; // String to store the time from the YR API response


// Display initialization
using DisplayType = GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT>;
DisplayType display(GxEPD2_213_BN(EPD_CS, EPD_DC, EPD_RSET, EPD_BUSY));

// Helper functions
String convertTime(const String& inputTime, const char* inputFormat, bool adjustTimezone = true) {
    struct tm tm;
    if (strptime(inputTime.c_str(), inputFormat, &tm) != NULL) {
        time_t t = mktime(&tm);
        if (adjustTimezone) {
            t += 2 * 3600; // Add 2 hours for UTC+2
        }
        struct tm *local_tm = localtime(&t);

        char timeStringBuff[20]; // Reduced buffer size as the new format is shorter
        strftime(timeStringBuff, sizeof(timeStringBuff), "%d.%m.%y %H:%M", local_tm);
        return String(timeStringBuff);
    }
    return "Failed to parse time";
}


// Main functions
void setup() {
    Serial.begin(115200);
    display.init();
    display.setRotation(3);
    display.setFont(&FreeSans9pt7b);
    display.setTextColor(GxEPD_BLACK);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    updateAndSleep();
}

void loop() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
        while (digitalRead(BUTTON_PIN) == LOW) {
            delay(10);
        }
    }

    updateAndSleep();
}

void updateAndSleep() {
    setupWiFi();

    if (fetchPrecipitationData()) {
        updateDisplayWithNewData();
    } else {
        Serial.println("Failed to fetch data");
    }

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    esp_sleep_enable_timer_wakeup(UPDATE_INTERVAL);
    esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(BUTTON_PIN), LOW);
    esp_deep_sleep_start();
}

void updateDisplayWithNewData() {
    Serial.println("Updating data...");
    if (fetchPrecipitationData()) {
        Serial.println("Data updated successfully");
        display.setFullWindow();
        display.firstPage();
        do {
            drawGraph(display, 16, 25, 225, 78, precipitationData, 18, "Nedbor neste 90 minutt", createdTime);
        } while (display.nextPage());
    } else {
        Serial.println("Failed to update data");
    }
}

void checkButtonAndUpdate() {
    if (digitalRead(BUTTON_PIN) == LOW) {  // Button is active LOW
        delay(50);  // Simple debounce
        if (digitalRead(BUTTON_PIN) == LOW) {
            updateDisplayWithNewData();
            while (digitalRead(BUTTON_PIN) == LOW) {
                delay(10);  // Wait for button release
            }
        }
    }
}

void setupWiFi() {
    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
    }

    if (DEBUG && WiFi.status() == WL_CONNECTED) {
        displayWiFiStatus(display, true, WiFi.localIP());
    }
}

bool fetchPrecipitationData() {
    HTTPClient http;
    http.begin(yrApiUrl);
    int httpResponseCode = http.GET();

    if (httpResponseCode != 200) {
        Serial.printf("HTTP response code: %d\n", httpResponseCode);
        http.end();
        return "";
    }

    String payload = http.getString();
    http.end();

    if (payload.isEmpty()) {
        return false;
    }

    return parsePrecipitationData(payload);
}

bool parsePrecipitationData(const String& payload) {
    DynamicJsonDocument doc(16384);
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
        Serial.println("JSON parsing failed");
        return false;
    }

    // Get the created time from the YR response
    String created = doc["created"].as<String>();
    createdTime = convertTime(created, "%Y-%m-%dT%H:%M:%S%Z", false);
    Serial.println("Adjusted local created time: " + createdTime);

    JsonArray points = doc["points"];
    Serial.println("Precipitation data:");
    for (int i = 0; i < 18 && i < points.size(); i++) {
        precipitationData[i] = points[i]["precipitation"]["intensity"];
        Serial.print(i * 5);
        Serial.print(" min: ");
        Serial.println(precipitationData[i]);
    }
    return true;
}