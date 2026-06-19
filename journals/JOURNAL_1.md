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

##### Breadboard wiring:

##### XIAO ESP32-S3 Sense → breadboard, power via USB-C
##### Hosyond display → jumper wires from its 14-pin header to XIAO's SPI pins (CS, DC, RESET, MOSI, SCK, MISO) + touch I2C pins
##### Camera is already attached to the XIAO Sense via its ribbon — no extra wiring needed for that part!

##### Then for software:

##### Get the display showing something basic (Arduino IDE + TFT_eSPI library)
##### Get the camera capturing photos (ESP32 camera library, built into Sense board support)
##### Add WiFi — connect to your network
##### Build your custom web app/API that the XIAO talks to
##### Display fetched data on the touchscreen, send photos to your server
