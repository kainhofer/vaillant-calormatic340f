# vaillant-calormatic340f

Helper files (utilities, data/signal dumps in decoded form, dead-end approached, etc.) for my decoding project of the Vailland calorMatic 340f wireless central heating control. The documentation of the protocol (i.e. the results obtained with these tools) can be found at http://wiki.kainhofer.com/hardware/vaillantvrt340f.

## Quick summary of the results
The remote control sends only the following data to the boiler, all other smart logic resides in the wireless controll (i.e. smart control, dumb boiler):
  * Heating On/Off (in analogue mode: heating water temperature)
  * Warm water pre-heating On/Off
  * Battery OK/LOW
![Packet structure of the Vaillant calorMatic 340f](https://github.com/kainhofer/vaillant-calormatic340f/blob/master/Images/Vaillant_CalorMatic340f_PacketStructure_Vertical.png)

All bytes are converted to a bit sequence with least-significant bit first.

For transmission over the 868,275MHz frequency, the data contents (bytes 4-14, but NOT bytes 3 and 15) are bit-stuffed (i.e. after five consecutive one bits, one extra zero bit is inserted). The resulting bit sequence is then encoded using differential Manchester encoding (1 is encoded as a transition, 0 as no transition) based on an underlying square wave of frequency 303Hz. Each bit period will be a half-wavelength, i.e. there will be 606 bit periods per second. After each bit period, a state transition is guaranteed and does not contain any information, and in the middle of each bit period there will be a state transition for a binary 1 and no transition or binary 0. 

Each signal is first sent with byte 9 set to 0x00 and shortly afterwards repeated with byte 9 set to 0x01 (and the checksum updated correspondingly).

![The signal and its repeat with comments](https://github.com/kainhofer/vaillant-calormatic340f/blob/master/Images/Vaillant-Thermostat-AM-868.287MHz-Wave2a-Zoom_Repeat_Annotated2.png)
