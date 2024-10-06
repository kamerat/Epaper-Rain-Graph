void displayWiFiStatus(DisplayType& display, bool isConnected, IPAddress ipAddress) {
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeSans9pt7b);

        display.setCursor(10, 30);
        if (isConnected) {
            display.print("WiFi connected");
            display.setCursor(10, 60);
            display.print("IP: ");
            display.print(ipAddress);
        } else {
            display.print("WiFi connection failed");
        }
    } while (display.nextPage());
}
