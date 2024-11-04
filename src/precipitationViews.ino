// Function to draw the precipitation graph
void drawPrecipitationGraph(DisplayType& display, int x, int y, int w, int h, const float* data, int dataSize, const char* title, const String& timeStr) {
    display.fillScreen(GxEPD_WHITE);

    drawTitle(display, x, y, title);
    drawTime(display, x, y, timeStr);
    drawAxes(display, x, y, w, h);
    drawGridLines(display, x, y, w, h);
    drawRaindrops(display, x, y, h);
    drawGraphData(display, x, y, w, h, data, dataSize);
    drawXAxisLabels(display, x, y, w, h);
}

// Function to draw the "No Precipitation" view
void drawNoPrecipitationView(DisplayType& display, const String& timestamp) {
    display.setFont(&FreeSans9pt7b);
    display.setCursor(10, 30);
    display.print("Det blir opphald");
    display.setCursor(10, 50);
    display.print("dei neste 90 minutta");

    display.setFont();
    display.setCursor(10, 70);
    display.print("Oppdatert: ");
    display.print(timestamp);
}

// Function to draw the "No Radar" view
void drawNoRadarView(DisplayType& display, const String& timestamp) {
    display.setFont(&FreeSans9pt7b);
    display.setCursor(10, 30);
    display.print("Ingen radardata");
    display.setCursor(10, 50);
    display.print("tilgjengelig");

    // Draw radar icon in top right
    int size = 40;
    int x = display.width() - size - 10; // 10 pixels from right edge
    int y = 10; // 10 pixels from top
    int centerX = x + size / 2;
    int centerY = y + size / 2;

    // Draw outer circle
    display.drawCircle(centerX, centerY, size / 2, GxEPD_BLACK);

    // Draw middle circle
    display.drawCircle(centerX, centerY, size / 3, GxEPD_BLACK);

    // Draw inner circle (slightly spaced from center)
    display.drawCircle(centerX, centerY, size / 5, GxEPD_BLACK);

    // Draw center dot
    display.fillCircle(centerX, centerY, 2, GxEPD_BLACK);

    // Draw vertical line
    display.drawLine(centerX, centerY, centerX, y + size, GxEPD_BLACK);

    // Draw 3 small dots spread around
    display.fillCircle(centerX - size / 4, centerY - size / 6, 1, GxEPD_BLACK);
    display.fillCircle(centerX + size / 5, centerY - size / 5, 1, GxEPD_BLACK);
    display.fillCircle(centerX + size / 6, centerY + size / 4, 1, GxEPD_BLACK);

    display.setFont();
    display.setCursor(10, 70);
    display.print("Oppdatert: ");
    display.print(timestamp);
}
