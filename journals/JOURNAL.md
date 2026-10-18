# June 19: Get components

Kind of want to start with a prototype
components:

- touchscreen
- xiao esp32 (mcu & internet)
- camera module (not built-in, want to customize the position in final phone)

I want to wire everything together/breadboard it
and then I can work on the software/whatever website API thing I'll build that looks like the slap.com
| some inspo |
| -- |
| ![the slap.com from victorious](https://static.wikia.nocookie.net/victorious/images/b/b4/PearPhone-horizontal.png/revision/latest/scale-to-width-down/1200?cb=20110825192536) |

what I'll buy now:
| item | purpose | cost |
| -- | -- | -- |
| [Hosyond 2.8" 240x320 IPS Capacitive Touch Screen](https://www.amazon.com/Hosyond-240x320-Capacitive-ILI9341V-Mega2560/dp/B0CMGF4Q68/ref=sr_1_4_sspa?crid=1XAGBEJ69NWKT&keywords=hoysound%2Blcd&qid=1781877174&sprefix=%2Caps%2C289&sr=8-4-spons&sp_csd=d2lkZ2V0TmFtZT1zcF9tdGY&th=1) | touch screen, not the built in circuit board one | $16.99 |
| [Seeed Studio XIAO ESP32S3 Sense Camera Sensor ESP32 S3 Dev Board](https://www.aliexpress.us/item/3256810060131517.html?spm=a2g0o.productlist.main.3.26c97W2u7W2uRJ&algo_pvid=9600b499-bb27-4ffa-8288-06fe5e6a5301&algo_exp_id=9600b499-bb27-4ffa-8288-06fe5e6a5301-2&pdp_ext_f=%7B%22order%22%3A%22139%22%2C%22eval%22%3A%221%22%2C%22fromPage%22%3A%22search%22%7D&pdp_npi=6%40dis%21USD%2176.25%2120.95%21%21%2176.25%2120.95%21%40210328d417818773112274592edf68%2112000051668143429%21sea%21US%217544465919%21ABX%211%210%21n_tag%3A-29910%3Bd%3Ae058c3f9%3Bm03_new_user%3A-29895%3BpisId%3A5000000207262906&curPageLogUid=YEOo9tZLpkHo&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005010246446269%7C_p_origin_prod%3A) | camera module, MCU, wifi capabilities | $20.95 |

I'll order these two parts, connect them physically, figure out the wifi and connect to websites
Then start building the slap website

## claude's advice

## Breadboard wiring:

## XIAO ESP32-S3 Sense → breadboard, power via USB-C
## Hosyond display → jumper wires from its 14-pin header to XIAO's SPI pins (CS, DC, RESET, MOSI, SCK, MISO) + touch I2C pins
## Camera is already attached to the XIAO Sense via its ribbon — no extra wiring needed for that part!

## Then for software:

## Get the display showing something basic (Arduino IDE + TFT_eSPI library)
## Get the camera capturing photos (ESP32 camera library, built into Sense board support)
## Add WiFi — connect to your network
## Build your custom web app/API that the XIAO talks to
## Display fetched data on the touchscreen, send photos to your server


# July 12: Schematic for pear-shaped PCB
## XIAO ESP32 board arrived, wanted to try programming and wiring it up to the touch screen/prototyping.

---

## Wiring up

## Mount across the center trench so pins don't short and leave wi-fi antenna extending over edge for stable connection

![Example wiring](/images/WiringExample.png)

## Using ESP32 dev kit as an example

![XIAO S3 GPIO pin layout](/images/XIAOS3GPIOS.png)

## Remapping SCK, MOSI (SDI), and MISO (SDO) to specific GPIO's on S3, other display pins will route to any free GPIOs

| display pin | fx                                               | XIAO S3 pin | Notes                                                                                                   |
| ----------- | ------------------------------------------------ | ----------- | ------------------------------------------------------------------------------------------------------- |
| SCK         | timing signal that synchronizes transfer of data | D8          | serial clock                                                                                            |
| SDI (MOSI)  | data input                                       | D10         | serial digital interface, protocol for transmitting uncompressed & unencrypted digital video over cable |
| SDO (MISO)  | data from peripheral                             | D9          | serial data out                                                                                         |

## Remaining pins (free GPIOs)

| display | c3  | fx  |
| ------- | --- | --- |
| LCD_CS  | D6  | ?   |
| LCD_RST | D5  | ?   |
| LCD_RS  | D4  | ?   |
| LED     | D7  | ?   |

## This doesn't include the touch screen pins because I need more male to female wires, I'll do that after I buy more. Got more and wired up:

![Example touch wiring](/images/TouchWiringExample.png)

| touch   | esp32 | c3  | fx            |
| ------- | ----- | --- | ------------- |
| CTP_SCL | 33    | 8   | Touch Sensing |
| CTP_RST | 13    | 9   | MISO          |
| CTP_SDA | 32    | ?   | GPIO          |
| CTP_INT | 34    | ?   | input only    |

## Oh apparently I can just wire the touch screen pins to any GPIOs.

| touch   | c3  | Notes         |
| ------- | --- | ------------- |
| CTP_SCL | 0   | Touch sensing |
| CTP_RST | 2   | MISO          |
| CTP_SDA | 1   | GPIO          |
| CTP_INT | 3   | Input only    |

## I guess SD_CS stays unconnected

## Programming

## Install ESP32 boots package Version 2

## Arduino IDE/Settings/Additional boards manager URL and insert [this link](https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json) then tools/board manager, search ESP32 and download version 2.0.4 by espressif. Once downloaded, select tools/board/esp32/XIAO_ESP32C3 (scroll down a lot)

## Install LovyanGFX 1.2.19, new tab and create an lgfx_config.h file

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

## It's compiling and uploading fine but not actually printing anything. I checked my wiring. Oh it's because my pinout is mapped to the S3 not the C3, no need to rewire, just update the code.. didn't know D-numbers and GPIO numbers were two separate naming systems.

## Change offset_rotation in the config file, 0 for portrait and 3 for landscape.

## For more complex UI, use the [advanced library](https://github.com/lvgl/lvgl)

![Press me demo](/images/PressMeDemo.JPG)

## I was like why isn't it registering my taps then realized I still had the plastic sticker on top 🙂‍↕️

## Can also import images via microSD card on back of touchscreen.

Going to start making the PCB (copper traces instead of jumper wires now)

Need to import some footprints:

[XIAO ESP32C3](https://www.snapeda.com/parts/XIAO%20ESP32C3/Seeed%20Studio/view-part/?ref=search&t=xiao%20esp32c3&ab_test_case=b)
\*Right now working with the C3 but getting the [S3](https://www.snapeda.com/parts/XIAO%20ESP32S3%20SENSE/Seeed%20Studio/view-part/?ref=search&t=xiao%20esp32s3%20sense&ab_test_case=b) (camera, so will have to update)

Can't find a footprint/symbol for the touchscreen, I'll place a 1x14, 2.54mm pitch female header (or box header) that the module's existing male pin header plugs into instead. See /production/imports

Not sure what type of connector: pin, generic, mounting pin, or generic shield. Pins stick out, generics are the black rectangular housing (probably what I want), supplies electricity, mounting is just structural/doesn't supply electricity, and shield is grounded

This is cool because before I didn't know the D-numbers and GPIO numbers were two separate naming systems but I can see it clearly in the schematic:
![XIAO Schematic](/images/XIAO_Sch.png)

Also if I want to add buttons/switches I'm going to need more pins, so the C3 may have not been the right MCU after all. I'm going to try routing the LED (backlight) to 5V and open up the D7 pin/8, then connect the switch to it. I've attached a power flag to the VUSB and GND pins of the C3 in the schematic (getting power externally) this resolves any "input power pin not driven by any output power pin" errors. This works for power and GND because when you plug into a USB port, you're getting both the supply and the return path from that one connection.

Because I'm adding a switch/push, I'll include in firmware

```
pinMode(pin, INPUT_PULLUP)
```

Cool, here's the schematic
![Schematic](/images/C3_Simple_Schematic.png)

Made the edgecut and pear phone logo /images/Silkscreens

I'll assign footprints next, I already imported the C3.

I'll use the switch/push buttons in the arduino kit, not sure if that's considered through hole (THT) or surface mount (SMD). I'm pretty sure if I was getting it assembled with the PCB I'd select SMD but because I'm mounting my own I'll go with THT? THT is the right choice, it physically matches the 6mm x 6mm switches I have, and is also easier for less "professional soldering". SMD switches actually look different, with flat pads/gull-wing leads that sit on the board's surface. I'll assign SW_PUSH to SW_PUSH_6mm.

Lastly we'll assign the connector pins to Connector_PinSocket_2.54mm:PinSocket_1x14_P2.54mm_Vertical

Pin socket is for female, pin header is for male, P2.54mm matches the datasheet's "14P 2.54mm spacing Header". I'll choose vertical for now so I can put the touchscreen in landscape but in the next variation I might choose vertical and put it in portrait mode.

Time to PCB!

I need to make sure my edge cuts/PCB's pear shape fits the display nicely. I'll add the pear logo to the back by setting it to B.Silkscreen, I might remake it with a filled in pear instead of the outline. To cut out the PCB I have to import it as a DXF, but first I need to fill in the actual pear shape so I'll quickly do that for both. See /images/onPCB. Cool I'll just make sure the header pins will size the touchscreen correctly, its 1" x 2.8" so 25.4mm x 71.1mm, I'll use the ruler in PCB viewer and make sure it fits on the whole board and on the headers nicely. I'm going to flip the board/pear shape horizontally to look better and make it smaller. I'll wire it up tomorrow.

| PCB                           | front                         | back                          |
| ----------------------------- | ----------------------------- | ----------------------------- |
| ![PCB now](/images/PCBv1.png) | ![PCB now](/images/3Dv1F.png) | ![PCB now](/images/3Dv1B.png) |

# July 13: Wired PCB

I'm going to try setting up the wifi server from the C3. I've plugged it in and am trying the WifiServer example sketch and have this [tutorial](https://www.youtube.com/watch?v=aH3sLEQI4_w&t=21s) open. I don't really care about turning on/off the LED, I just want to see if it connects to the web server, I might connect it by toggling on/off the backlight LED of the touchscreen.

I kept trying to upload the WifiServer sketch and it wouldn't work, then remembered the C3 came with another piece that's like a connector, I'll try adding it and running. So it's a Ipex connector connects the WiFi, bluetooth antenna to increase signal strength, so the code should be running without it but it may help to add. Sweet it worked and printed my the C3's IP.

```
#include <Arduino.h>
#include <WiFi.h>

const char *ssid = "***";
const char *password = "***";

NetworkServer server(80);

void setup() {
  Serial.begin(115200);

  delay(10);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  NetworkClient client = server.accept();  // listen for incoming clients

  if (client) {                     // if you get a client,
    Serial.println("New Client.");  // print a message out the serial port
    String currentLine = "";        // make a String to hold incoming data from the client
    while (client.connected()) {    // loop while the client's connected
      if (client.available()) {     // if there's bytes to read from the client,
        char c = client.read();     // read a byte, then
        Serial.write(c);            // print it out the serial monitor
        if (c == '\n') {            // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 5 on.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 5 off.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {  // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
```

If I want to make an actual website (maybe use [this tumblr website](https://theslapdotgov.tumblr.com/) as a reference) I'll create the website files within the C3 project instead of putting it in another project and directing to its domain. Instead of using a specific IP address I could also look into using mDNS for friendlier links.

# October 18: Playing with GFX
# June 19: 

Time to wire the PCB, I think I'll buy it off of PCBWay because they can do a minimum of 1 PCB, I don't want 5 from JLCPCB. I'm just not sure how they want the fabrication files.

Cool she's wired, no errors, just silkscreen warnings we can ignore and complaints about the footprint I'm pretty sure I resolved before. Let's get it on PCBWay. Wait I kind of want an outlined of the pear shape like what I did on the blinky.. maybe on V2.
![v1 PCB Wiring](/images/v1PCBWiring.png)

Looks like min quantity on PCBWay is also 5. Back to JLCPCB.

This is the current BOM, the PCB is $7.60 and $11.72 with Global Standard Shipping and taxes

| Id  | Designator | Footprint                       | Quantity | Designation  | Supplier and ref |
| --- | ---------- | ------------------------------- | -------- | ------------ | ---------------- |
| 1   | LCD1       | PinSocket_1x14_P2.54mm_Vertical | 1        | Conn_01x14   |                  |
| 2   | SW1        | SW_PUSH_6mm                     | 1        | SW_Push      |                  |
| 3   | U2         | MODULE_XIAO_ESP32C3             | 1        | XIAO_ESP32C3 |                  |

I remember having to edit some of the column names for the devboard, hopefully I didn't have to do that for this/KiCad's BOM will work directly, especially if I'm not asking them to assemble anything.
