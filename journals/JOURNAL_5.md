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
