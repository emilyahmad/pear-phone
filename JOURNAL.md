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

##### claude's advice

Breadboard wiring:

XIAO ESP32-S3 Sense → breadboard, power via USB-C
Hosyond display → jumper wires from its 14-pin header to XIAO's SPI pins (CS, DC, RESET, MOSI, SCK, MISO) + touch I2C pins
Camera is already attached to the XIAO Sense via its ribbon — no extra wiring needed for that part!

Then for software:

Get the display showing something basic (Arduino IDE + TFT_eSPI library)
Get the camera capturing photos (ESP32 camera library, built into Sense board support)
Add WiFi — connect to your network
Build your custom web app/API that the XIAO talks to
Display fetched data on the touchscreen, send photos to your server

---

find footprints & symbols on snap eda:

- 3d models (for render)

on pcb =

XIAO ESP32S3

- was originally going to use the ESP32-S3-WROOM-1
- that way I don't have to add a USB-C connector, AMS, and additional capacitors and resistors
- small ~ 21 x 7 mm

external =
Hosyond touchscreen — plugs into 14-pin header
OV2640 camera — plugs into FPC connector

"ESP32-S3-WROOM-1" (module, not bare chip; that way don't have to connect extra stuff like crystal osc)
source:
https://www.snapeda.com/parts/ESP32-S3-WROOM-1-N8R8/Espressif%20Systems/view-part/?company=GE&ref=search&t=ESP32-S3-WROOM-1&ab_test_case=b

2.8" TFT part number = Hosyond 2.8 inch 240x320 IPS Capacitive Touch Screen SPI Serial ILI9341V Driver LCD Display Module for Arduino R3/Mega2560
source: https://www.amazon.com/Hosyond-240x320-Capacitive-ILI9341V-Mega2560/dp/B0CMGF4Q68/ref=sr_1_10?crid=3IAEYY18MLUUX&dib=eyJ2IjoiMSJ9.oWGL-Z1tWnxU4akI8CPv6_OE_DFxJiuCUXws_5zAb9DxvPdFLY2xWZ6CA6VUsIu5pqy9hhBk3XTG1Thq4PAsAilsWm9_5F3a_JyP058ZoBgirSeAlgJcJVOXxJ-aThQibyBA89HPCAJA5VcbKiqjz0ITCV-3L8N3m8-tlI1ysdikxSICUSU5b8ay9LSVAB_oKvQ0cxphAx4SVpLS9QEXwApRu_qqTQ8TjkBsPqv57jY.P4MHjI3tJXGjmmgT0HRU1f4T_dwDHrV_TVhm92gng0M&dib_tag=se&keywords=2.8%22%2Btft%2Btouchscreen&qid=1778099441&sprefix=2.8%2Btft%2Btouchscreen%2Caps%2C172&sr=8-10&th=1
==========
display uses a 14P 2.54mm header connector (Symbol: Connector_PinHeader_2.54mm → PinHeader_1x14
Footprint: Connector_PinHeader_2.54mm → PinHeader_1x14_Vertical)

GET OV2640 CAMERA MODULE
OV2640 module comes on a small PCB with the lens attached and connects via ribbon cable. You don't need a symbol for it — you just need the FPC connector on your PCB. The camera module plugs into that.
source: https://www.amazon.com/JESSINIE-Monitoring-Identification-Connector-Support/dp/B0B5XGMTSZ/ref=sr_1_3?crid=6Y4KLZ2RFKEF&dib=eyJ2IjoiMSJ9.jYmoOPxuV18LwzrOehhU6_01AFgZtVBH74ZlLSdiP1F2nq1MTnePX0XULQm0U_IIZlY34YhvGHBH92uQMFrWNpuzdF3YESX95OYwqb6tOhg4JudoADMetEFvHmPQLwXifxRufseuxSAMIJ6aa9zrV5CoBiTLjMcs2Z0hjMmTCX_dsP_ZmTNjB0sReyZnZwCqMUQ20ezYIZhGWO6QOGgxSRpojskz1uUP1rTdS2Ut89g.ghiB4MQsy2_JhbDOVgofcUOo_6lQ4BPbS7leoUiZtkk&dib_tag=se&keywords=OV2640+camera+module+24-pin+FPC+ribbon+cable+included&nsdOptOutParam=true&qid=1778099796&sprefix=ov2640+camera+module+24-pin+fpc+ribbon+cable+included%2Caps%2C231&sr=8-3

"FPC connector 24 pin" (OV2640 typically uses 24-pin)
conn_01x24 symbol & fpc 24 pin 0.5mm footprint
source: https://www.snapeda.com/parts/FH12-24S-0.5SH%2855%29/Hirose/view-part/?ref=search&t=FPC%2024%20pin%200.5mm&ab_test_case=b

"USB4105" or any USB-C connector
source: https://www.snapeda.com/parts/USB4105GFA/Global%20Connector%20Technology/view-part/?ref=search&t=USB4105&ab_test_case=b

"AMS1117-3.3"
source: https://www.snapeda.com/parts/AMS1117-3.3/UMW/view-part/?ref=search&t=AMS1117-3.3&ab_test_case=b

schematic

Power:
USB-C → AMS1117-3.3 → VCC/GND rails
ESP32-S3-WROOM-1:

VCC, GND
SPI pins → TFT screen
I2C pins → touch controller
Camera pins → FPC connector
GPIO → reset button
EN pin → 10k resistor to VCC

==============================================

# [ON PCB]

esp32 (mcu)
usb-c
cc resistors for usb-c connector (2 5.1k ohm resistors connect from cc1 and cc2 to GND)
ams (voltage regulator)
hirose (camera ribbon plugs into)
pin header 01x14 (plug in display)
push (reset button)
decoupling ccapacitors (.1uF & 10uF near AMS1117)
10k ohm resistor for esp32 pin pullup

# [BUYING AS SEPARATE COMPONENTS, WILL CONNECT ONCE SHIPPED]

camera module (connects to ribbon via fpc )
display (plugs into 14pin connector)

======================================================
