Hardware source files for TBIdeas lamp
======================================

This github project contains the source file for the different revisions and prototypes of our smart lamp called Illumi and then Loochi.

## License

    Copyright 2012 - TBIdeas.com
    Distributed under the CC-BY-SA license.
    http://creativecommons.org/licenses/by-sa/3.0/

## v0.0

This is the simplest way to drive an RGB light. Those files were produced to prepare a [blog post introducing development for Illumi][blog-v0.0].

## v0.1

This is a very cheap to build linear current source for high power RGB LEDs. Find out more and read more on the [blog article][blog-v0.1]. 

## v0.2

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

## Let's stay in touch!

We hope you will find this information useful! We are open-sourcing all of this in the hope that it will be useful to someone else - if it is, or if you have any question, please contact us: contact THE_AROBASE tbideas.com.


[blog-v0.0]: http://www.tbideas.com/blog/illumi-101-iphone-controlled-rgb-light/
[blog-v0.1]: http://www.tbideas.com/blog/build-an-arduino-shield-to-drive-high-power-rgb-led/
[stackexchange-avcc]: http://electronics.stackexchange.com/questions/36167/understanding-the-avcc-pin-wiring-on-arduinoleonardo-low-pass-filter
[blog-v0.2]: http://www.tbideas.com/blog/some-pictures-of-our-latest-prototype/
[blog-board3d]: http://www.tbideas.com/blog/design-of-the-led-driver/
[contact]: contact@tbideas.com

