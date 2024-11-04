# Lilygo-T5 Rain Graph

Weather precipitation graph using Lilygo T5 2.13" ESP32 E-paper screen. This project displays a precipitation forecast for the next 90 minutes using data from YR.no.

<img src="https://github.com/user-attachments/assets/b2aef714-5bec-43eb-8240-26e3a8b5919c" alt="drawing" width="450"/>

## Hardware Requirements

- [Lilygo T5 2.13" ESP32 E-paper display](https://s.click.aliexpress.com/e/_EvVOHHF)

## Software Setup

1. Install [VS Code](https://code.visualstudio.com/)

2. Install [PlatformIO IDE](https://platformio.org/install/ide?install=vscode) extension for VS Code

3. Clone this repository

4. Create a `src/config.h` file based on the provided `src/example.config.h`

5. Open the project in VS Code:
   - File > Open Folder > Select the project folder
   - PlatformIO will automatically install all required dependencies defined in `platformio.ini`

6. Customize your configuration in `src/config.h`:
   - Modify the `USER_AGENT` and add a link to your GitHub profile to properly authenticate with YR.no
   - Set `SHOW_GRAPH_ON_NO_PRECIPITATION` to liking
   - Set `SHOW_BATTERY_INDICATOR` to liking

7. Build and Upload:
   - Click the PlatformIO "Upload" button
   - Select the appropriate port when prompted

## Usage

Once uploaded, the device will prompt you to connect to Wi-Fi. Once connected, it will fetch precipitation data from YR.no, and display a graph showing the precipitation forecast for the next 90 minutes.

## Development

The project uses PlatformIO for dependency management and building. Key dependencies include:
- QRcodeDisplay
- Adafruit GFX Library
- GxEPD2
- WiFiManager
- ArduinoJson
- Time

## Contributing

Feel free to open issues or submit pull requests to improve this project.

## License

This work is licensed under [Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International](https://creativecommons.org/licenses/by-nc-sa/4.0/)

## Attributions

- Weather data provided by the Norwegian Meteorological Institute. Used under the [CC BY 4.0 license](https://creativecommons.org/licenses/by/4.0/).
