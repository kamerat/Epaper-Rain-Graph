#define LILYGO_T5_V213
#include <boards.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include "config.h" // Environment variables

// Constants
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* yrApiUrl = "https://www.yr.no/api/v0/locations/1-211102/forecast/now";
const int BUTTON_PIN = 39; // LilyGo T5 integrated button pin

// Global variables
float precipitationData[18]; // Array to store 90 minutes of precipitation data (5-minute intervals)

// Display initialization
GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> display(GxEPD2_213_BN(EPD_CS, EPD_DC, EPD_RSET, EPD_BUSY));

// Function definitions follow...

void updateDisplayWithNewData() {
    Serial.println("Updating data...");
    if (fetchPrecipitationData()) {
        Serial.println("Data updated successfully");
        display.setFullWindow();
        display.firstPage();
        do {
            drawGraph(6, 5, 235, 114, precipitationData, 18, "Nedbor neste 90 minutt");
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
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(10, 30);
        display.print("Connecting to WiFi...");
        display.display();
    } while (display.nextPage());

    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
    }

    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(10, 30);
        if (WiFi.status() == WL_CONNECTED) {
            display.print("WiFi connected");
            display.setCursor(10, 60);
            display.print("IP: ");
            display.print(WiFi.localIP());
        } else {
            display.print("WiFi connection failed");
            delay(2000); // Display the Wi-Fi status for 2 seconds
        }
    } while (display.nextPage());
}

bool fetchPrecipitationData() {
    HTTPClient http;
    http.begin(yrApiUrl);
    http.addHeader("User-Agent", USER_AGENT_PERSONAL);

    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
        String payload = http.getString();
        Serial.println("API Response:");
        Serial.println(payload);

        DynamicJsonDocument doc(16384);
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
            Serial.println("JSON parsing failed");
            return false;
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
    } else {
        Serial.print("HTTP request failed, error: ");
        Serial.println(httpResponseCode);
        return false;
    }
}

void drawGraph(int x, int y, int w, int h, float* data, int dataSize, const char* title) {
    display.fillScreen(GxEPD_WHITE);

    display.setFont(&FreeSans9pt7b);

    // Draw title
    display.setCursor(x, y + 12);
    display.print(title);

    // Adjust graph position and size
    y += 20;
    h -= 36; // Reduce height to accommodate title and labels

    // Draw axes
    display.drawLine(x, y + h, x + w, y + h, GxEPD_BLACK); // X-axis

    // Set fixed max value for y-axis
    float maxVal = 5.0; // 5 mm/h as max precipitation

    // Draw horizontal grid lines
    int lineY1 = y + h * 1/4;  // Top line
    int lineY2 = y + h * 2/4;
    int lineY3 = y + h * 3/4;
    display.drawLine(x, lineY1, x + w, lineY1, GxEPD_BLACK);
    display.drawLine(x, lineY2, x + w, lineY2, GxEPD_BLACK);
    display.drawLine(x, lineY3, x + w, lineY3, GxEPD_BLACK);

    // Plot data points and fill area under the curve
    for (int i = 0; i < dataSize - 1; i++) {
        int x1 = x + (i * w / (dataSize - 1));
        int y1 = y + h - (data[i] * h / maxVal);
        int x2 = x + ((i + 1) * w / (dataSize - 1));
        int y2 = y + h - (data[i + 1] * h / maxVal);

        // Fill area under the curve
        display.fillTriangle(x1, y1, x2, y2, x1, y + h, GxEPD_BLACK);
        display.fillTriangle(x2, y2, x2, y + h, x1, y + h, GxEPD_BLACK);

        // Draw line connecting points
        display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);
    }

    // Add x-axis labels and ticks
    const char* labels[] = {"No", "15", "30", "45", "60", "75", "90"};
    for (int i = 0; i < 7; i++) {
        int labelX = x + (i * w / 6);

        // Draw tick
        display.drawLine(labelX, y + h, labelX, y + h + 5, GxEPD_BLACK);

        // Draw label
        if (i < 6) {
            display.setCursor(labelX - 8, y + h + 18);
            display.print(labels[i]);
        } else {
            // Offset the "90" label
            display.setCursor(labelX - 10, y + h + 18);
            display.print("90");
        }
    }

    // // Add y-axis labels
    // display.setCursor(x - 20, y + 10);
    // display.print(maxVal, 1);
    // display.setCursor(x - 20, y + h);
    // display.print("0");
}

void setup() {
    Serial.begin(115200);
    display.init();
    display.setRotation(3); // Change this from 1 to 3 for 180-degree rotation
    display.setFont(&FreeSans9pt7b);  // Set the default font to the smaller one
    display.setTextColor(GxEPD_BLACK);

    setupWiFi();

    Serial.println("Fetching precipitation data...");
    if (fetchPrecipitationData()) {
        Serial.println("Data fetched successfully");
    } else {
        Serial.println("Failed to fetch data");
    }

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    updateDisplayWithNewData();  // Initial data fetch and display
}

void loop() {
    checkButtonAndUpdate();
    delay(100);  // Small delay to prevent excessive checking
}
