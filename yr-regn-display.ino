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
#include <WiFiManager.h>

// Constants
const char* yrApiUrl = "https://www.yr.no/api/v0/locations/" YR_LOCATION "/forecast/now";
const int BUTTON_PIN = 39; // LilyGo T5 integrated button pin
const int UPDATE_INTERVAL = 5 * 60 * 1000000;  // 5 minutes in microseconds

// Global variables
float precipitationData[18]; // Array to store 90 minutes of precipitation data (5-minute intervals)
String responseTime; // String to store the time from the HTTP response
String createdTime; // String to store the time from the YR API response
bool radarIsDown = false; // Flag to track if the radar is down

// Display initialization
using DisplayType = GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT>;
DisplayType display(GxEPD2_213_BN(EPD_CS, EPD_DC, EPD_RSET, EPD_BUSY));

// Main functions
void setup() {
    Serial.begin(115200);
    display.init();
    display.setRotation(3);
    display.setFont(&FreeSans9pt7b);
    display.setTextColor(GxEPD_BLACK);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    WiFiManager wifiManager;

    wifiManager.setConfigPortalTimeout(300);

    if (DEBUG) {
        wifiManager.resetSettings();
    }

    const char* randomWifiPassword = generatePassword();

    wifiManager.setAPCallback([&](WiFiManager* wifiManager) {
        displayWiFiSetup(display, wifiManager, randomWifiPassword);
    });

    if (wifiManager.autoConnect("regnvarsel", randomWifiPassword)) {
        Serial.println("WiFi connected");
        updateAndSleep();
    } else {
        Serial.println("Failed to connect and hit timeout");
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        displayWiFiFailedSleep(display);
        esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(BUTTON_PIN), LOW);
        esp_deep_sleep_start();
    }
}


void loop() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
        while (digitalRead(BUTTON_PIN) == LOW) {
            delay(10);
        }
        updateAndSleep();
    }
}

void updateAndSleep() {
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
            if (radarIsDown) {
                drawNoRadarView(display, createdTime);
            } else {
                // Check if there's any precipitation data
                bool hasPrecipitation = false;
                for (int i = 0; i < 18; i++) {
                    if (precipitationData[i] > 0) {
                        hasPrecipitation = true;
                        break;
                    }
                }

                if (hasPrecipitation || SHOW_GRAPH_ON_NO_PRECIPITATION) {
                    drawPrecipitationGraph(display, 16, 25, 225, 78, precipitationData, 18, "Nedbor neste 90 minutt", createdTime);
                } else {
                    drawNoPrecipitationView(display, createdTime);
                }
            }
        } while (display.nextPage());
    }
}

bool fetchPrecipitationData() {
    HTTPClient http;
    http.begin(yrApiUrl);
    int httpResponseCode = http.GET();

    if (httpResponseCode != 200) {
        Serial.printf("HTTP response code: %d\n", httpResponseCode);
        displayHTTPError(display, httpResponseCode);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    if (payload.isEmpty()) {
        displayDataFetchError(display);
        return false;
    }

    return parsePrecipitationData(payload);
}

// Helper function
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

bool parsePrecipitationData(const String& payload) {
    DynamicJsonDocument doc(16384);
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
        Serial.println("JSON parsing failed");
        displayJSONParsingError(display);
        return false;
    }

    // Get the created time from the YR response
    String created = doc["created"].as<String>();
    createdTime = convertTime(created, "%Y-%m-%dT%H:%M:%S%Z", false);
    Serial.println("Adjusted local created time: " + createdTime);

    // Check if radar is down
    radarIsDown = doc["radarIsDown"].as<bool>();
    if (radarIsDown) {
        Serial.println("Radar is down");
        return true; // We still return true because we got a valid response
    }

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

const char* generatePassword() {
    static const char* passwords[] = {
        "regnbukse", "paraply", "solskinn", "skybrudd",
        "duskregn", "plaskedam", "takrenne", "regnskyll",
        "regnbuer", "flomregn", "lynogtorden"
    };
    int arraySize = sizeof(passwords) / sizeof(passwords[0]);
    int randomIndex = random(0, arraySize);
    return passwords[randomIndex];
}