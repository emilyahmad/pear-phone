Going to start making the PCB (copper traces instead of jumper wires now)

Need to import some footprints:

[XIAO ESP32C3](https://www.snapeda.com/parts/XIAO%20ESP32C3/Seeed%20Studio/view-part/?ref=search&t=xiao%20esp32c3&ab_test_case=b)
\*Right now working with the C3 but getting the [S3](https://www.snapeda.com/parts/XIAO%20ESP32S3%20SENSE/Seeed%20Studio/view-part/?ref=search&t=xiao%20esp32s3%20sense&ab_test_case=b) (camera, so will have to update)

Can't find a footprint/symbol for the touchscreen, I'll place a 1x14, 2.54mm pitch female header (or box header) that the module's existing male pin header plugs into instead. See /production/imports

Not sure what type of connector: pin, generic, mounting pin, or generic shield. Pins stick out, generics are the black rectangular housing (probably what I want), supplies electricity, mounting is just structural/doesn't supply electricity, and shield is grounded

This is cool because before I didn't know the D-numbers and GPIO numbers were two separate naming systems but I can see it clearly in the schematic:
![XIAO Schematic](/images/XIAO_Sch.png)

Also if I want to add buttons/switches I'm going to need more pins, so the C3 may have not been the right MCU after all. I'm going to try routing the LED (backlight) to 5V and open up the D7 pin/8, then connect the switch to it.

Because I'm adding a switch/push, I'll include in firmware

```
pinMode(pin, INPUT_PULLUP)
```

Cool, here's the schematic
![Schematic](/images/C3_Simple_Schematic.png)
