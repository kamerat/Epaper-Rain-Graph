#include "qrcodeeink.h"

void displayWiFiSetup(DisplayType& display, WiFiManager *wifiManager, const char* randomWifiPassword) {
    QRcodeEink qrcode(&display);
    qrcode.init(-70);
    do {
        // Generate QR code for WiFi configuration
        String qrData = "WIFI:S:" + String(wifiManager->getConfigPortalSSID()) + ";T:WPA;P:" + String(randomWifiPassword) + ";;";
        qrcode.create(qrData);

        display.setFont();
        display.setTextColor(GxEPD_BLACK);

        display.setCursor(115, 15);
        display.print("WIFI Konfigurasjon");
        display.setCursor(115, 25);
        display.print(FW_VERSION);
        display.setCursor(115, 49);
        display.print("SSID: ");
        display.setCursor(115, 79);
        display.print("Passord: ");

        display.setFont(&FreeSans9pt7b);
        display.setCursor(115, 69);
        display.print(wifiManager->getConfigPortalSSID());
        display.setCursor(115, 101);
        display.print(randomWifiPassword);
    } while (display.nextPage());
}

void displayWiFiSetupFailed(DisplayType& display) {
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        display.setFont();

        // draw zzz
        display.setFont(&FreeSans9pt7b);
        display.setCursor(80, 45);
        display.print("z");

        display.setCursor(100, 35);
        display.print("z");

        display.setCursor(120, 25);
        display.print("z");

        // Draw the message
        display.setCursor(10, 80);
        display.print("WiFi konfigurasjon feilet");
        display.setCursor(10, 95);
        display.setFont();
        display.print("Restart enheten for a prove paa nytt");
    } while (display.nextPage());
}

void displayWiFiLostMessage(DisplayType& display, int retryTime) {
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setFont(&FreeSans9pt7b);
        display.setCursor(10, 30);
        display.print("Mistet WiFi-tilkobling");
        display.setFont();
        display.setCursor(10, 50);
        display.print("Prover igjen om ");
        display.drawLine(23, 55, 23 + 3, 55 - 3, GxEPD_BLACK);
        display.print(retryTime);
        display.print(" minutter.");
        display.setCursor(10, 80);
        display.print("Eller hold nede knappen i 3 sekunder");
        display.setCursor(10, 90);
        display.print("mens du starter opp enheten for a");
        display.drawPixel(display.getCursorX() - 4, 90, GxEPD_BLACK);
        display.setCursor(10, 100);
        display.print("nullstille WiFi-innstillinger.");
    } while (display.nextPage());
}
