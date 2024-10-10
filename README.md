# Lilygo-T5 Rain graph

Weather precipitation graph using Lilygo T5 2.13" ESP32 E-paper screen. This project displays a precipitation forecast for the next 90 minutes using data from YR.no.

<img src="https://github.com/user-attachments/assets/b2aef714-5bec-43eb-8240-26e3a8b5919c" alt="drawing" width="450"/>

## Hardware Requirements

- Lilygo T5 2.13" ESP32 E-paper display

## Software Setup

1. Install the Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software)

2. Add ESP32 board support to Arduino IDE:
   - Open Arduino IDE
   - Go to File > Preferences
   - Add the following URL to the "Additional Boards Manager URLs" field:
     `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Go to Tools > Board > Boards Manager
   - Search for "esp32" and install the "ESP32 by Espressif Systems" package

3. Install required libraries:
   - Download libs from https://github.com/Xinyuan-LilyGO/LilyGo-T5-Epaper-Series/tree/master/lib and put them in `Documents\Arduino\libraries`
   - Go to Sketch > Include Library > Manage Libraries
   - Search for and install the following libraries:
     - Time
     - ArduinoJson
     - WiFiManager
     - QRcodeDisplay

5. Clone this repository or download the `yr-regn-display.ino` file

6. Create a `config.h` file in the same directory as `yr-regn-display.ino` based on the `example.config.h` file

7. Open the `yr-regn-display.ino` file in Arduino IDE

8. Customize your configuration:
   - In the `config.h` file:
     - Modify the `USER_AGENT` if desired
     - Set the `YR_LOCATION` to your desired location ID
     - Set `DEBUG` to `true` or `false` as needed
     - set `SHOW_GRAPH_ON_NO_PRECIPITATION` to liking

9. Select the correct board and port:
   - Go to Tools > Board and select "ESP32 Dev Module" or your specific Lilygo T5 board
   - Go to Tools > Port and select the appropriate port for your device

10. Upload the sketch to your Lilygo T5 device

## Usage

Once uploaded, the device will prompt you to connect to Wi-Fi. Once connected, it will fetch precipitation data from YR.no, and display a graph showing the precipitation forecast for the next 90 minutes.

## Contributing

Feel free to open issues or submit pull requests to improve this project.

## License

This work is licensed under [Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International](https://creativecommons.org/licenses/by-nc-sa/4.0/)

## Attributions

- Weather data provided by the Norwegian Meteorological Institute. Used under the [CC BY 4.0 license](https://creativecommons.org/licenses/by/4.0/).
