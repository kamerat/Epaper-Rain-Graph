void displayError(DisplayType& display, const char* errorMessage) {
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeSans9pt7b);

        display.setCursor(10, 30);
        display.print("Error:");

        // Split the error message into multiple lines if needed
        int16_t x1, y1;
        uint16_t w, h;
        int yPos = 60;
        String message = errorMessage;
        while (message.length() > 0) {
            String line = "";
            int lastSpace = -1;
            for (int i = 0; i < message.length(); i++) {
                line += message[i];
                if (message[i] == ' ') {
                    display.getTextBounds(line.c_str(), 0, 0, &x1, &y1, &w, &h);
                    if (w > display.width() - 20) {
                        line = line.substring(0, lastSpace);
                        break;
                    }
                    lastSpace = i;
                }
            }
            line.trim();
            display.setCursor(10, yPos);
            display.print(line);
            yPos += 25;
            message = message.substring(line.length());
            message.trim();
        }
    } while (display.nextPage());
}

void displayWiFiError(DisplayType& display) {
    displayError(display, "WiFi connection failed. Please check your credentials and try again.");
}

void displayHTTPError(DisplayType& display, int httpResponseCode) {
    char errorMessage[100];
    if (httpResponseCode == -1) {
        snprintf(errorMessage, sizeof(errorMessage), "HTTP request failed due to network issue. Please check your connection.");
    } else {
        snprintf(errorMessage, sizeof(errorMessage), "HTTP request failed. Response code: %d", httpResponseCode);
    }
    displayError(display, errorMessage);
}

void displayJSONParsingError(DisplayType& display) {
    displayError(display, "Failed to parse JSON data. Please check the API response format.");
}

void displayDataFetchError(DisplayType& display) {
    displayError(display, "Failed to fetch precipitation data. Please try again later.");
}
