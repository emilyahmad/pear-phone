# pear-phone


# good tutorials
[firmware](https://www.youtube.com/watch?v=GGGEvKVZpJQ)

# components

(features)
2.8" tft touchscreen (capacitive)
ESP32-S3-WROOM-1
Push button — for reset at minimum, useful for your ESP32

(camera)
OV2640 camera module connected via ribbon cable (FPC connector)
FPC connector (the socket on your PCB that the camera ribbon plugs into)

(power)
usb c connector
& AMS1117-3.3 voltage regulator (3.3V for ESP32)

(other)
decoupling capacitors

v2/later on (portability)
USB-C to charge a small LiPo battery via a TP4056 module
flash LED (for night photos)