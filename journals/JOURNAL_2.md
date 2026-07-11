### XIAO ESP32 board arrived, wanted to try programming and wiring it up to the touch screen/prototyping.

---

#### Wiring up

#### Mount across the center trench so pins don't short and leave wi-fi antenna extending over edge for stable connection

![Example wiring](/images/WiringExample.png)

#### Using ESP32 dev kit as an example

![XIAO S3 GPIO pin layout](/images/XIAOS3GPIOS.png)

#### Remapping SCK, MOSI (SDI), and MISO (SDO) to specific GPIO's on S3, other display pins will route to any free GPIOs

| display pin | fx                                               | XIAO S3 pin | Notes                                                                                                   |
| ----------- | ------------------------------------------------ | ----------- | ------------------------------------------------------------------------------------------------------- |
| SCK         | timing signal that synchronizes transfer of data | D8          | serial clock                                                                                            |
| SDI (MOSI)  | data input                                       | D10         | serial digital interface, protocol for transmitting uncompressed & unencrypted digital video over cable |
| SDO (MISO)  | data from peripheral                             | D9          | serial data out                                                                                         |

#### Remaining pins (free GPIOs)

| display | c3  | fx  |
| ------- | --- | --- |
| LCD_CS  | D6  | ?   |
| LCD_RST | D5  | ?   |
| LCD_RS  | D4  | ?   |
| LED     | D7  | ?   |

#### This doesn't include the touch screen pins because I need more male to female wires, I'll do that after I buy more. Got more and wired up:

![Example touch wiring](/images/TouchWiringExample.png)

| touch   | esp32 | c3  | fx            |
| ------- | ----- | --- | ------------- |
| CTP_SCL | 33    | 8   | Touch Sensing |
| CTP_RST | 13    | 9   | MISO          |
| CTP_SDA | 32    | ?   | GPIO          |
| CTP_INT | 34    | ?   | input only    |

#### Oh apparently I can just wire the touch screen pins to any GPIOs.

| touch   | c3  | Notes         |
| ------- | --- | ------------- |
| CTP_SCL | 0   | Touch sensing |
| CTP_RST | 2   | MISO          |
| CTP_SDA | 1   | GPIO          |
| CTP_INT | 3   | Input only    |

#### I guess SD_CS stays unconnected

### Programming

#### Install ESP32 boots package Version 2

#### Arduino IDE/Settings/Additional boards manager URL and insert [this link](https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json) then tools/board manager, search ESP32 and download version 2.0.4 by espressif. Once downloaded, select tools/board/esp32/XIAO_ESP32C3 (scroll down a lot)

#### Install LovyanGFX 1.2.19, new tab and create an lgfx_config.h file

```
// LovyanGFX custom config for:
//   Board:   Seeed XIAO ESP32-C3
//   Display: Hosyond 2.8" IPS 240x320, ILI9341V driver, 4-line SPI
//   Touch:   FT6336G capacitive, I2C
//
// Pin mapping used (XIAO D-label -> real ESP32-C3 GPIO):
//   LCD_RS(DC) D4 -> GPIO6    LCD_RST D5 -> GPIO7    LCD_CS  D6 -> GPIO21
//   LED(BL)    D7 -> GPIO20   SCK     D8 -> GPIO8    MISO    D9 -> GPIO9
//   MOSI       D10 -> GPIO10
//   CTP_SCL    D0 -> GPIO2    CTP_SDA D1 -> GPIO3    CTP_RST D2 -> GPIO4
//   CTP_INT    D3 -> GPIO5

#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI        _bus_instance;
  lgfx::Light_PWM      _light_instance;
  lgfx::Touch_FT5x06   _touch_instance;   // covers FT6206/FT6236/FT6336/FT6436

public:
  LGFX(void)
  {
    { // ---- SPI bus (shared by the display) ----
      auto cfg = _bus_instance.config();

      cfg.spi_host   = SPI2_HOST;    // ESP32-C3: use SPI2_HOST or SPI3_HOST (not VSPI/HSPI)
      cfg.spi_mode   = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read  = 16000000;
      cfg.spi_3wire  = false;        // this module has a real MISO line, so 4-wire SPI
      cfg.use_lock   = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = 8;              // D8 (SCK)
      cfg.pin_mosi = 10;             // D10 (SDI/MOSI)
      cfg.pin_miso = 9;              // D9 (SDO/MISO)
      cfg.pin_dc   = 6;              // D4 (LCD_RS)

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    { // ---- Panel ----
      auto cfg = _panel_instance.config();

      cfg.pin_cs  = 21;              // D6 (LCD_CS)
      cfg.pin_rst = 7;               // D5 (LCD_RST)
      cfg.pin_busy = -1;

      cfg.panel_width      = 240;
      cfg.panel_height     = 320;
      cfg.offset_x         = 0;
      cfg.offset_y         = 0;
      cfg.offset_rotation  = 3;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits  = 1;
      cfg.readable         = true;
      cfg.invert           = true;    // required for IPS panels (this one is IPS)
      cfg.rgb_order        = false;   // set true if red/blue look swapped
      cfg.dlen_16bit        = false;
      cfg.bus_shared         = true;

      _panel_instance.config(cfg);
    }

    { // ---- Backlight ----
      auto cfg = _light_instance.config();

      cfg.pin_bl      = 20;           // D7 (LED)
      cfg.invert      = false;
      cfg.freq        = 44100;
      cfg.pwm_channel = 0;            // C3 only has LEDC channels 0-5, not 7

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    { // ---- Touch (FT6336G, I2C, addr 0x38) ----
      auto cfg = _touch_instance.config();

      cfg.x_min = 0;
      cfg.x_max = 239;
      cfg.y_min = 0;
      cfg.y_max = 319;
      cfg.pin_int    = 5;             // D3 (CTP_INT)
      cfg.pin_rst    = 4;             // D2 (CTP_RST)
      cfg.bus_shared = false;         // touch is on its own I2C bus, not the display's SPI bus
      cfg.offset_rotation = 0;

      cfg.i2c_port = 1;               // either free I2C port (0 or 1) works
      cfg.i2c_addr = 0x38;            // FT6336G default address
      cfg.pin_sda  = 3;               // D1 (CTP_SDA)
      cfg.pin_scl  = 2;               // D0 (CTP_SCL)
      cfg.freq     = 400000;

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }
};
```

Run to print Hello World

```
#include <LovyanGFX.h>
#include "lgfx_config.h"

LGFX tft;

void setup() {
  Serial.begin(115200);
  delay(50);

  tft.init();
  tft.setBrightness(255);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 20);

  tft.println("Hello, world!");
  tft.println("LovyanGFX on ESP32");
}

void loop() {
  static uint32_t last = 0;
  static int x = 20;

  if (millis() - last > 250) {
    last = millis();
    tft.fillRect(20, 80, 240, 24, TFT_BLACK);
    tft.setCursor(20, 80);
    tft.print("Tick ");
    tft.print(millis() / 1000);
    tft.print("s");
  }
}
```

#### It's compiling and uploading fine but not actually printing anything. I checked my wiring. Oh it's because my pinout is mapped to the S3 not the C3, no need to rewire, just update the code.. didn't know D-numbers and GPIO numbers were two separate naming systems

#### Change offset_rotation in the config file, 0 for portrait and 3 for landscape.

#### For more complex UI, use the [advanced library](https://github.com/lvgl/lvgl)

![Press me demo](/images/PressMeDemo.JPG)

#### I was like why isn't it registering my taps then realized I still had the plastic sticker on top 🙂‍↕️

#### Can also import images via microSD card on back of touchscreen.
