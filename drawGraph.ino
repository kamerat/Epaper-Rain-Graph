const float MAX_PRECIPITATION_INTENSITY = 17.0;

float coordinateFromSquaredNowPrecipitationIntensity(float value, int maxHeight) {
    if (value >= MAX_PRECIPITATION_INTENSITY) {
        return maxHeight - 20; // TODO: Fix the maxHeight to be 58 instead of 78 while keeping the graph bottom aligned
    }

    float maxPrecipitationIntensitySquared = sqrt(MAX_PRECIPITATION_INTENSITY);
    float valueSquared = sqrt(value);

    return round((valueSquared / maxPrecipitationIntensitySquared) * maxHeight);
}

void drawGraph(DisplayType& display, int x, int y, int w, int h, const float* data, int dataSize, const char* title, const String& timeStr) {
    display.fillScreen(GxEPD_WHITE);

    drawTitle(display, x, y, title);
    drawTime(display, x, y, timeStr);
    drawAxes(display, x, y, w, h);
    drawGridLines(display, x, y, w, h);
    drawRaindrops(display, x, y, h);
    drawGraphData(display, x, y, w, h, data, dataSize);
    drawXAxisLabels(display, x, y, w, h);
}

void drawTitle(DisplayType& display, int x, int y, const char* title) {
    display.setFont(&FreeSans9pt7b);
    display.setCursor(x, y - 8);
    display.print(title);

    // As font does not support ø, hack it by adding a slash
    display.drawLine(x + 51, y - 18, x + 43, y - 7, GxEPD_BLACK);
    display.drawLine(x + 52, y - 18, x + 44, y - 8, GxEPD_BLACK);
}

void drawTime(DisplayType& display, int x, int y, const String& timeStr) {
    display.setFont();
    display.setTextSize(1);
    display.setCursor(x, y - 2);
    display.print(timeStr);
}

void drawAxes(DisplayType& display, int x, int y, int w, int h) {
    display.drawLine(x, y + h, x + w, y + h, GxEPD_BLACK); // X-axis
}

void drawGridLines(DisplayType& display, int x, int y, int w, int h) {
    int lineY1 = y + h * 1/4;
    int lineY2 = y + h * 2/4;
    int lineY3 = y + h * 3/4;
    display.drawLine(x, lineY1, x + w, lineY1, GxEPD_BLACK);
    display.drawLine(x, lineY2, x + w, lineY2, GxEPD_BLACK);
    display.drawLine(x, lineY3, x + w, lineY3, GxEPD_BLACK);
}

void drawRaindrops(DisplayType& display, int x, int y, int h) {
    int lineY1 = y + h * 1/4;
    int lineY2 = y + h * 2/4;
    int lineY3 = y + h * 3/4;
    drawRaindrop(display, x - 9, lineY1, 3);
    drawRaindrop(display, x - 9, lineY2, 2);
    drawRaindrop(display, x - 9, lineY3, 1);
}

void drawGraphData(DisplayType& display, int x, int y, int w, int h, const float* data, int dataSize) {
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

void drawXAxisLabels(DisplayType& display, int x, int y, int w, int h) {
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

void drawRaindrop(DisplayType& display, int x, int y, int fillLevel) {
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
