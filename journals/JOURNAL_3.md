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
