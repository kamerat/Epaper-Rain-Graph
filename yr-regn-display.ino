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
const float MAX_PRECIPITATION_INTENSITY = 17.0;

// Global variables
float precipitationData[18]; // Array to store 90 minutes of precipitation data (5-minute intervals)
String responseTime; // String to store the time from the HTTP response
String createdTime; // String to store the time from the YR API response


// Display initialization
GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> display(GxEPD2_213_BN(EPD_CS, EPD_DC, EPD_RSET, EPD_BUSY));

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

float coordinateFromSquaredNowPrecipitationIntensity(float value, int maxHeight) {
    if (value >= MAX_PRECIPITATION_INTENSITY) {
        return maxHeight - 20; // TODO: Fix the maxHeight to be 58 instead of 78 while keeping the graph bottom aligned
    }

    float maxPrecipitationIntensitySquared = sqrt(MAX_PRECIPITATION_INTENSITY);
    float valueSquared = sqrt(value);

    return round((valueSquared / maxPrecipitationIntensitySquared) * maxHeight);
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
            drawGraph(16, 25, 225, 78, precipitationData, 18, "Nedbor neste 90 minutt", createdTime);
        } while (display.nextPage());
    } else {
        Serial.println("Failed to update data");
    }
}

void drawGraph(int x, int y, int w, int h, float* data, int dataSize, const char* title, String timeStr) {
    display.fillScreen(GxEPD_WHITE);

    drawTitle(x, y, title);
    drawTime(x, y, timeStr);
    drawAxes(x, y, w, h);
    drawGridLines(x, y, w, h);
    drawRaindrops(x, y, h);
    drawGraphData(x, y, w, h, data, dataSize);
    drawXAxisLabels(x, y, w, h);
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
        display.setFullWindow();
        display.firstPage();
        do {
            display.fillScreen(GxEPD_WHITE);
            display.setCursor(10, 30);
            display.print("WiFi connected");
            display.setCursor(10, 60);
            display.print("IP: ");
            display.print(WiFi.localIP());
        } while (display.nextPage());
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

void drawTitle(int x, int y, const char* title) {
    display.setFont(&FreeSans9pt7b);
    display.setCursor(x, y - 8);
    display.print(title);

    // As font does not support ø, hack it by adding a slash
    display.drawLine(x + 51, y - 18, x + 43, y - 7, GxEPD_BLACK);
    display.drawLine(x + 52, y - 18, x + 44, y - 8, GxEPD_BLACK);
}

void drawTime(int x, int y, String timeStr) {
    display.setFont();
    display.setTextSize(1);
    display.setCursor(x, y - 2);
    display.print(timeStr);
}

void drawAxes(int x, int y, int w, int h) {
    display.drawLine(x, y + h, x + w, y + h, GxEPD_BLACK); // X-axis
}

void drawGridLines(int x, int y, int w, int h) {
    int lineY1 = y + h * 1/4;
    int lineY2 = y + h * 2/4;
    int lineY3 = y + h * 3/4;
    display.drawLine(x, lineY1, x + w, lineY1, GxEPD_BLACK);
    display.drawLine(x, lineY2, x + w, lineY2, GxEPD_BLACK);
    display.drawLine(x, lineY3, x + w, lineY3, GxEPD_BLACK);
}

void drawRaindrops(int x, int y, int h) {
    int lineY1 = y + h * 1/4;
    int lineY2 = y + h * 2/4;
    int lineY3 = y + h * 3/4;
    drawRaindrop(x - 9, lineY1, 3);
    drawRaindrop(x - 9, lineY2, 2);
    drawRaindrop(x - 9, lineY3, 1);
}

void drawGraphData(int x, int y, int w, int h, float* data, int dataSize) {
    int coordinates[dataSize + 1][2];

    // Calculate coordinates
    for (int i = 0; i < dataSize; i++) {
        int minute = i * 5;
        coordinates[i][0] = x + (minute * w / 90);
        coordinates[i][1] = y + h - coordinateFromSquaredNowPrecipitationIntensity(data[i], h);
    }
    coordinates[dataSize][0] = x + w;
    coordinates[dataSize][1] = coordinates[dataSize - 1][1];

    // Draw and fill graph
    display.fillTriangle(x, y + h, coordinates[0][0], coordinates[0][1], x, coordinates[0][1], GxEPD_BLACK);
    for (int i = 0; i < dataSize; i++) {
        display.fillTriangle(
            coordinates[i][0], coordinates[i][1],
            coordinates[i+1][0], coordinates[i+1][1],
            coordinates[i][0], y + h,
            GxEPD_BLACK
        );
        display.fillTriangle(
            coordinates[i+1][0], coordinates[i+1][1],
            coordinates[i+1][0], y + h,
            coordinates[i][0], y + h,
            GxEPD_BLACK
        );
        display.drawLine(coordinates[i][0], coordinates[i][1], coordinates[i+1][0], coordinates[i+1][1], GxEPD_BLACK);
    }
}

void drawXAxisLabels(int x, int y, int w, int h) {
    const char* labels[] = {"No", "15", "30", "45", "60", "75", "90"};
    for (int i = 0; i < 7; i++) {
        int labelX = x + (i * w / 6);
        display.drawLine(labelX, y + h, labelX, y + h + 5, GxEPD_BLACK);
        display.setFont();
        display.setTextSize(1);
        if (i == 0) {
            display.setCursor(labelX, y + h + 7);
        } else if (i < 6) {
            display.setCursor(labelX - 5, y + h + 7);
        } else {
            display.setCursor(labelX - 10, y + h + 7);
        }
        display.print(labels[i]);
    }
}

void drawRaindrop(int x, int y, int fillLevel) {
    const int size = 6;
    int radius = size / 2;

    // Draw the raindrop shape (circle + triangle)
    display.drawCircle(x, y, radius, GxEPD_BLACK);
    display.drawTriangle(x - radius, y, x + radius, y, x, y - size, GxEPD_BLACK);

    // Fill the drop based on the fillLevel
    if (fillLevel == 3) {
        display.fillCircle(x, y, radius, GxEPD_BLACK);
        display.fillTriangle(x - radius, y, x + radius, y, x, y - size, GxEPD_BLACK);
    } else if (fillLevel == 2) {
        display.fillCircle(x, y, radius, GxEPD_BLACK);
        display.drawLine(x - radius + 3, y - radius,     x + radius - 3, y - radius,     GxEPD_WHITE);
        display.drawLine(x - radius + 2, y - radius + 1, x + radius - 2, y - radius + 1, GxEPD_WHITE);
        display.drawLine(x - radius + 2, y - radius + 2, x + radius - 2, y - radius + 2, GxEPD_WHITE);
    } else if (fillLevel == 1) {
        display.fillCircle(x, y, radius, GxEPD_BLACK);
        display.drawLine(x - radius + 3, y - radius,     x + radius - 3, y - radius,     GxEPD_WHITE);
        display.drawLine(x - radius + 2, y - radius + 1, x + radius - 2, y - radius + 1, GxEPD_WHITE);
        display.drawLine(x - radius + 1, y - radius + 2, x + radius - 1, y - radius + 2, GxEPD_WHITE);
        display.drawLine(x - radius + 1, y - radius + 3, x + radius - 1, y - radius + 3, GxEPD_WHITE);
        display.drawPixel(x, y - radius + 4, GxEPD_WHITE);
    }
}