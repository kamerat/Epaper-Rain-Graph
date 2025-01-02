#define LILYGO_T5_V213
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
#include <Preferences.h>

// Constants
const char* yrApiUrl = "https://www.yr.no/api/v0/locations/%s/forecast/now";
const int BUTTON_PIN = 39; // LilyGo T5 integrated button pin
const int UPDATE_INTERVAL = 5 * 60 * 1000000;  // 5 minutes in microseconds
const int CYCLES_BEFORE_RESTART = 288; // Restart every 24 hours (288 * 5 minutes)
const char* FW_VERSION = "v1.4.0"; // Version number - Just for showing in the wifi setup screen

// Global variables
String yrLocation; // String to store the YR location provided by the user
float precipitationData[18]; // Array to store 90 minutes of precipitation data (5-minute intervals)
String responseTime; // String to store the time from the HTTP response
String createdTime; // String to store the time from the YR API response
bool radarIsDown = false; // Flag to track if the radar is down
Preferences prefs; // Long-term persistence storage for YR location

// RTC memory variables
RTC_DATA_ATTR int updateCycles = 0; // Persist across deep sleep

// Display initialization
using DisplayType = GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT>;
DisplayType display(GxEPD2_213_BN(5, 17, 16, 4));

WiFiManager wifiManager;

// Main functions
void setup() {
    Serial.begin(115200);
    display.init();
    display.setRotation(3);
    display.setFont(&FreeSans9pt7b);
    display.setTextColor(GxEPD_BLACK);

    // Only show splash screen on power-on reset
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED) {
        displaySplashScreen(display);
        delay(1000);
    }

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    wifiManager.setConfigPortalTimeout(300);

    prefs.begin("regnvarsel", false);
    yrLocation = prefs.getString("location", "");
    WiFiManagerParameter custom_yr_location("yr_location", "YR Location ID", yrLocation.c_str(), 40);
    wifiManager.addParameter(&custom_yr_location);

    // Add instructions for the YR location ID in the WiFi setup
    WiFiManagerParameter custom_text("<p style=\"margin:0;font-size:11px;\">Find the location ID by searching for your location on yr.\u200Bno and copying the \"x-xxxxx\" ID from the URL. (e.g. yr.\u200Bno/nn/vêrvarsel/dagleg-tabell/<strong>1-92416</strong>/...)</p>");
    wifiManager.addParameter(&custom_text);

    // Set styling and redirect directly to the wifi setup page
    wifiManager.setCustomHeadElement("<style>button{background-color:#000;}.msg,h1{display:none;}</style>"
                                     "<script>"
                                     "if(!localStorage.getItem('visited')){"
                                     "localStorage.setItem('visited','true');"
                                     "window.location.href='/wifi';"
                                     "}"
                                     "</script>");

    const char * menu[] = {"wifi","info"};
    wifiManager.setMenu(menu, 2);

    // Check for 3sec button press to reset device
    checkButtonPressForReset(wifiManager);

    const char* randomWifiPassword = generatePassword();

    wifiManager.setAPCallback([&](WiFiManager* wifiManager) {
        if (wifiManager->getWiFiIsSaved()) {
            handleWiFiConnectionFailure();
        } else {
            displayWiFiSetup(display, wifiManager, randomWifiPassword);
        }
    });


    //set custom ip for portal
    wifiManager.setAPStaticIPConfig(IPAddress(10,0,0,1), IPAddress(10,0,0,1), IPAddress(255,255,255,0));

    if (wifiManager.autoConnect("Regnvarsel", randomWifiPassword)) {
        Serial.println("WiFi connected");
        yrLocation = custom_yr_location.getValue();

        if (yrLocation.length() == 0) {
            displayError(display, "YR location not set. Please configure in WiFi settings");
            delay(5000);
            wifiManager.resetSettings();
            prefs.clear();
            ESP.restart();
        } else {
            prefs.putString("location", yrLocation);
            updateAndSleep();
        }
    } else {
        Serial.println("Failed to connect and hit timeout");
        handleWiFiConnectionFailure();
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
    if (++updateCycles >= CYCLES_BEFORE_RESTART) {
        Serial.println("Restarting device due to update cycle limit");
        ESP.restart();
        return;
    }

    if (fetchPrecipitationData()) {
        updateDisplayWithNewData();
    }

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    esp_sleep_enable_timer_wakeup(UPDATE_INTERVAL);
    esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(BUTTON_PIN), LOW);
    esp_deep_sleep_start();
}

void updateDisplayWithNewData() {
    Serial.println("Updating display with new data...");
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

        if (SHOW_BATTERY_INDICATOR) {
            if (SHOW_GRAPH_ON_NO_PRECIPITATION) {
                drawBatteryIndicator(display, 110, 23);
            } else {
                drawBatteryIndicator(display, 10, 105);
            }
        }
    } while (display.nextPage());
}

bool fetchPrecipitationData() {
    HTTPClient http;
    char fullUrl[200];
    snprintf(fullUrl, sizeof(fullUrl), yrApiUrl, yrLocation.c_str());
    http.begin(fullUrl);
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
        Serial.println("Empty payload received from YR API");
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
        "regnbukse", "paraplyer", "solskinn", "skybrudd",
        "duskregn", "plaskedam", "takrenne", "regnskyll",
        "regnbuer", "flomregn", "lynogtorden"
    };
    int arraySize = sizeof(passwords) / sizeof(passwords[0]);
    int randomIndex = random(0, arraySize);
    return passwords[randomIndex];
}

void checkButtonPressForReset(WiFiManager& wifiManager) {
    if (digitalRead(BUTTON_PIN) == LOW) {
        unsigned long startTime = millis();
        while (digitalRead(BUTTON_PIN) == LOW) {
            if (millis() - startTime > 3000) {  // 3 seconds hold time
                display.setFullWindow();
                display.firstPage();
                do {
                    display.fillScreen(GxEPD_WHITE);
                    display.setCursor(10, 30);
                    display.print("Resetting device...");
                } while (display.nextPage());

                wifiManager.resetSettings();
                ESP.restart();
            }
        }
    }
}

void handleWiFiConnectionFailure() {
    WiFi.mode(WIFI_STA); // We need to be enable wifi first in order to get saved WiFi info via WiFiManager
    if (wifiManager.getWiFiIsSaved()) {
        // WiFi was previously configured but connection failed
        displayWiFiLostMessage(display, UPDATE_INTERVAL / 1000000 / 60);
        esp_sleep_enable_timer_wakeup(UPDATE_INTERVAL);
    } else {
        // No WiFi configured, fallback to splash screen
        displaySplashScreen(display);
    }

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(BUTTON_PIN), LOW);
    esp_deep_sleep_start();
}

float getBatteryVoltage() {
    float voltage = analogRead(35) / 4096.0 * 7.46;
    return voltage;
}

int getBatteryPercentage(float voltage) {
    if (voltage >= BATTERY_MAX_VOLTAGE) return 100;
    if (voltage <= BATTERY_MIN_VOLTAGE) return 0;
    return (voltage - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE) * 100;
}

void drawBatteryIndicator(DisplayType& display, int x, int y) {
    float voltage = getBatteryVoltage();
    display.setCursor(x, y);
    display.setFont();

    int percentage = getBatteryPercentage(voltage);
    Serial.printf("Battery: %d%% (%.2fV)\n", percentage, voltage);

    // Battery outline
    display.drawRect(x, y, 19, 8, GxEPD_BLACK);
    display.fillRect(x + 19, y + 2, 2, 4, GxEPD_BLACK);

    // Add text for battery percentage and voltage
    display.setCursor(x + 24, y);
    display.setTextSize(1);
    display.print(percentage);
    display.print("%");

    // Calculate battery level
    int level = map(percentage, 0, 100, 0, 18);
    display.fillRect(x + 1, y + 1, level, 6, GxEPD_BLACK);
}
