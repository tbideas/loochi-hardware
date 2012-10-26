Hardware source files for TBIdeas lamp
======================================

This github project contains the source file for the different revisions and prototypes of our smart lamp called Illumi and then Loochi.

## License

    Copyright 2012 - TBIdeas.com
    Distributed under the CC-BY-SA license.
    http://creativecommons.org/licenses/by-sa/3.0/

## Loochi v0.4

Third prototype.

This one is very similar to 0.3 but instead of a Bluegiga BLE112 module, we use an Aircable module with its generic firmware (so it does not need any programming). This module is not yet publicly available but will be very soon. Until then, you can contact us if you would like to get some.

In this design, we have also removed the npn/pnp transistors that were used to drive the mosfet. We wanted to compare the performance with and without them.

The communication protocol is labeled as I2C between the BLE module and the attiny but you can also do SPI. We have not really decided which one we will stick to.

### Known problems and limitations

This board was designed before we had time to assemble the 0.3 so the same problem applies on the LED pads, and connections.

## Loochi v0.3

Second complete prototype designed and built by us.

There is only one board with the LED soldered directly on the board. The PCB is intended to be an aluminium core PCB. For prototypes we have used traditional FR4 boards (from the awesome oshpark.com). In any case you absolutely need a pretty big heatsink on the other side of the board.

This board design is a major rework over the previous version. The main goal was to reduce manufacturing cost so that more people could enjoy Loochi. Another major design decision was to drop Wifi. We could write a lot about that but the biggest arguments were: (1) Wifi seriously increases the costs of the solution and (2) the efforts required by the user to set-up a Wifi lamp are important.

Main characteristics:
 * One ATTiny 861A is used to control the current going through the LED and adjust the brightness
 * One BLE112 module from Bluegiga is used for Bluetooth Low Energy communication with the app, it acts as an SPI Master to the Attiny
 * There is no more dedicated chip for the LED PWM (to reduce costs)
 * Ledengin 10W RGGB Led soldered directly on the PCB
 * Funny fact: the board radius is 3.1415925 cm

You will find more information on how this board works in the source code of the AVR. Several versions are provided (with graduating levels of complexity).

We also plan to do a video to explain how it works. If you are interested, let us know and it will probably be done quicker!

### Known problems on the board layout

There are a few known problems on this PCB:
 * The LED footprint is wrong: the first green LED is backwards (stupid mistake when linking pads and signals)
 * The schematic uses the RESET pin of the uController but we decided later on to change that, the mapping used in the software is BSENSE connected to PB5 and BPWM connected to PB4 (which means the Blue PWM output is inverted but allows us to keep the RESET pin free)
 * The BLE112 module is very sensitive to the quality of the power supply, add an RC filter on its VCC
 * Remove Q4 and short its source and drain (otherwise you will get a shortcut) - Warning: this removes the reverse voltage protection

If you want to make your own, we suggest that you fix those before getting the board manufactured or wait for a new design from us that would not have those problems. Another option is always to cut some traces and put some small wires. This is what we have done.

## Illumi v0.2

__Note: We used to call this version Illumi - dont be surprised in the documentation and source code__

This is the first complete prototype completely designed and built by us.

It is a dual-board design: a board with the micro-controller and the Wifi card, and another board with the LED drivers.

Features of the motherboard:

 * Arduino-Leonardo clone running at 8Mhz - Programmable via a ICSP connector or through the USB cable (once the bootloader has been burnt). You can use the Arduino IDE to program it.
 * Wifi module RN-171 from Roving Networks with a wire antenna
 * Bluetooth 4 Low Energy BLE112 module from Bluegiga
 * Only 75mm by 50mm
 * Switching 3.3 V power supply
 
Features of the LED driver

 * Input voltage from ~5V to 20V (depends on the type of LED you want to power) - Compatible with all sorts of high power LEDs
 * Three LT3518 swiching regulator for a 700mA constant current source to drive one RGB led
 * PWM inputs to control the brightness of each component: Red, Green, Blue
 * Shutdown pin to turn off the regulators and save power
 * Three temperature sensors to measure the board temperature

### Files

 * illumi-motherboard.sch: Eagle Schematic for the motherboard
 * illumi-motherboard.sch: Eagle Board for the motherboard
 * illumi-leddriver.sch: Eagle Schematic for the LED Driver
 * illumi-leddriver.sch: Eagle Board for the LED Driver
 * illumi-leddriver.asc: LTSpice simulation for one LT3518 channel 
 * illumi-v0.2-bom.csv: Bill of material - Detailed list of parts with Mouser reference number, price, etc

### Other resources

 * Source code for the motherboard is available in the Github project named "illumi"
 * Configuration files for the Arduino environment to program this board is in the project named "illumi-arduino"
 * Blog articles:
 ** [3D renderings][blog-board3d]
 ** [Pictures of the assembled board][blog-v0.2]

### Known problems and limitations

 * On the motherboard, you must not solder R16 and bridge the pads instead. The idea was to build a low-pass filter with R16 and C13 but the voltage drop on AVCC is too high and the chip is not programmable with R16 in place. Check out [this discussion on electronics stackexchange][stackexchange-avcc] for more info.
 * The LED driver includes a LM317 to provide 3.3V to the three LT3518. Although they dont use much current, bringing a 20V input to 3.3V incurs some serious heat dissipation on the LM317. It works so far but the next designs will use the motherboard voltage regulator for 3.3V

## Illumi v0.1

This is a very cheap to build linear current source for high power RGB LEDs. Find out more and read more on the [blog article][blog-v0.1]. 

## Illumi v0.0

This is the simplest way to drive an RGB light. Those files were produced to prepare a [blog post introducing development for Illumi][blog-v0.0].

## Let's stay in touch!

We hope you will find this information useful! We are open-sourcing all of this in the hope that it will be useful to someone else - if it is, or if you have any question, please contact us: contact THE_AROBASE tbideas.com.


[blog-v0.0]: http://www.tbideas.com/blog/illumi-101-iphone-controlled-rgb-light/
[blog-v0.1]: http://www.tbideas.com/blog/build-an-arduino-shield-to-drive-high-power-rgb-led/
[stackexchange-avcc]: http://electronics.stackexchange.com/questions/36167/understanding-the-avcc-pin-wiring-on-arduinoleonardo-low-pass-filter
[blog-v0.2]: http://www.tbideas.com/blog/some-pictures-of-our-latest-prototype/
[blog-board3d]: http://www.tbideas.com/blog/design-of-the-led-driver/
[contact]: contact@tbideas.com

