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
      cfg.offset_rotation = 2;

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