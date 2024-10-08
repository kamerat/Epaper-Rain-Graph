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