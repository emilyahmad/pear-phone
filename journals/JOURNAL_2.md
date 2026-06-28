#### Seeed Studio XIAO ESP32 board came, wanted to try programming it and wire it up to the touch screen

#### Install ESP32 boots package Version 2: navigate to Arduino IDE/Settings/Additional boards manager URL and insert [this link](https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json) then to tools/board manager, search ESP32 and download version 2.0.4 by espressif. Once downloaded, tools/board/esp32/XIAO_ESP32C3 (scroll far)

---

#### Wiring up

#### Mount across the center trench so pins don't short and leave wi-fi antenna extending over edge for stable connection

#### XIAO ESP32S3's GPIO pins are 3.3V logic (accepts 5V but signal pins should match mcu), so wire touschreen's VCC to 3.3V-OUT pin and GND (third and second on left respectively)

![Example wiring](/images/WiringExample.png)

#### using ESP32 dev kit as an example & remapping SCK, MOSI (SDI), and MISO (SDO) to specific GPIO's on S3, other display pins will route to any free GPIO's

| display pin | fx                                               | XIAO S3 pin | notes                                                                                                   |
| ----------- | ------------------------------------------------ | ----------- | ------------------------------------------------------------------------------------------------------- |
| SCK         | timing signal that synchronizes transfer of data | D8          | serial clock                                                                                            |
| SDI (MOSI)  | data input                                       | D10         | serial digital interface, protocol for transmitting uncompressed & unencrypted digital video over cable |
| SDO (MISO)  | data from peripheral                             | D9          | serial data out                                                                                         |

#### Remaining pins:

LCD_CS, LCD_RST, LCD_RS, LED