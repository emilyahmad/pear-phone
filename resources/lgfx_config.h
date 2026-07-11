/*******************************************************************************
 *  MostlyBuilds - ESP32 SPI Capacitive Touchscreen Tutorial
 *  Author: MostlyBuilds
 *  Guide:  https://mostlybuilds.com/esp32-spi-capacitive-touch-screen-tutorial
 *
 *  LovyangGFX (LGFX) Configuration
 *******************************************************************************/

#pragma once
#include <LovyanGFX.hpp>

// I2C Touch Pins
static constexpr int TOUCH_INT = 34; // CTP_INT
static constexpr int I2C_SDA = 32;   // CTP_SDA
static constexpr int TOUCH_RST = 13; // CTP_RST
static constexpr int I2C_SCL = 33;   // CTP_SCL

// Screen SPI pins
static constexpr int PIN_MISO = 9;   // SDO(MISO)
static constexpr int PIN_BL = 7;     // LED
static constexpr int PIN_SCLK = 8;   // SCK
static constexpr int PIN_MOSI = 10;  // SDI(MOSI)
static constexpr int PIN_DC = 4;     // LCD_RS
static constexpr int PIN_RST = 5;    // LCD_RST
static constexpr int PIN_LCD_CS = 6; // LCD_CS (SS)

// Change the screen dimensions to match your screen.
// Set this to the native resolution and use "offset_rotation"
// configurations to change the screen orientation
// rather than swapping these values to handle portrait
// versus landscape.

// Dimensions for the 2.8" screen used in this tutorial
static constexpr uint16_t SCREEN_WIDTH = 240;
static constexpr uint16_t SCREEN_HEIGHT = 320;

// Dimensions for the 3.5" screen used in this tutorial
// static constexpr uint16_t SCREEN_WIDTH = 320;
// static constexpr uint16_t SCREEN_HEIGHT = 480;

// Change this for your specific touchscreen controller
// 0x38 is a very common value, used by FocalTech FT5x06
// family (FT5206/FT5306/FT5406/FT6236/FT6336, etc.)
static constexpr int TOUCH_I2C_ADDRESS = 0x38;

struct LGFX : public lgfx::LGFX_Device
{

    // Panel instance for the 2.8" screen in this tutorial
    lgfx::Panel_ILI9341V _panel_instance;

    // Panel instnace for the 3.5" screen in this tutorial
    // lgfx::Panel_ST7796 _panel_instance;

    // Change to the correct touch instance for your touch controller
    // This works for both screens used in this tutorial
    lgfx::Touch_FT6336G _touch_instance;

    lgfx::Bus_SPI _bus_instance;
    lgfx::Light_PWM _light_instance;

    LGFX(void)
    {
        // Panel configuration
        {
            auto cfg = _panel_instance.config();

            cfg.pin_cs = PIN_LCD_CS;
            cfg.pin_rst = PIN_RST;
            cfg.pin_busy = -1;
            cfg.memory_width = SCREEN_WIDTH;
            cfg.memory_height = SCREEN_HEIGHT;
            cfg.panel_width = SCREEN_WIDTH;
            cfg.panel_height = SCREEN_HEIGHT;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 3; // 2.8" screen used in this tutorial
            // cfg.offset_rotation  = 1;  // 3.5" screen used in this tutorial
            //  Rotation for landscape with header pins
            //  on the right side of the screen

            cfg.invert = true; // Fixes the incorrect color issue
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits = 1;
            cfg.readable = true;
            cfg.rgb_order = false;
            cfg.bus_shared = true;
            cfg.dlen_16bit = false;

            _panel_instance.config(cfg);
        }

        // SPI bus configuration
        {
            auto cfg = _bus_instance.config();
            cfg.pin_sclk = PIN_SCLK;
            cfg.pin_mosi = PIN_MOSI;
            cfg.pin_miso = PIN_MISO;
            cfg.pin_dc = PIN_DC;
            cfg.spi_host = VSPI_HOST;
            cfg.freq_write = 40000000;
            cfg.freq_read = 10000000;
            cfg.dma_channel = 1;

            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        // Backlight configuration
        {
            auto cfg = _light_instance.config();

            cfg.pin_bl = PIN_BL; // LED backlight pin
            cfg.invert = false;
            cfg.freq = 1000;     // PWM frequency (1kHz is standard for LED backlight)
            cfg.pwm_channel = 0; // Use channel 0 instead of 7 to avoid conflicts

            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        // Touch config
        {
            auto cfg = _touch_instance.config();

            cfg.x_min = 0;
            cfg.x_max = SCREEN_WIDTH - 1;
            cfg.y_min = 0;
            cfg.y_max = SCREEN_HEIGHT - 1;
            cfg.offset_rotation = 2; // 2.8" screen used in this tutorial
            // cfg.offset_rotation = 0;  // 3.5" screen used in this tutorial
            //  Rotation for landscape with header pins
            //  on the right side of the screen
            cfg.pin_int = TOUCH_INT;
            cfg.pin_rst = TOUCH_RST;
            cfg.pin_sda = I2C_SDA;
            cfg.pin_scl = I2C_SCL;
            cfg.i2c_port = 0;
            cfg.i2c_addr = TOUCH_I2C_ADDRESS;
            cfg.freq = 400000;

            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }

        setPanel(&_panel_instance);
    }
};
