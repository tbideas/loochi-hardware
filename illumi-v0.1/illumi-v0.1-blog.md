# Build an Arduino shield to drive high-power RGB LED

If you have read our [previous post][previous-post], you know how easy it is to connect a RGB led to an Arduino micro-controller. It is amazing to put this thing in a corner of your room and change the color during the night but chances are ... you will be the only one impressed by that.

The [very basic LED][datasheet] we used in our previous post has a power of 45mW (go ahead, take the [datasheet][datasheet] and do the math, it is an interesting exercice, post your results below!) and generates less than one lumen of [luminous flux][lumen]. This is not enough to light anything so you will quickly want to use much more powerful LEDs.

In this post, we will see how to build a very cheap linear LED driver and plug it to our Arduino so that you can actually drive high power RGB LED with the same software we used last time. This time, it gets impressive!

<!--more-->

# Choosing an high-power LED

For a while, high power LEDs were 1W or 3W LEDs. Nowadays, you can easily find 10W, 40W or [90W][lzp]. These are the types of LEDs we will focus on today. Because the bigger the better, especially whey you are building interior lighting fixtures like [Illumi][illumi]. 

The most important characteristics to look at are:

* What is the forward voltage of the diode: You will need to supply the LED with at least that voltage;
* What is the forward current of the diode: This is the current that you will need to supply to the diode. This needs to be very carefully regulated because if you let the current go to high, it will burn the LED. That's why we will build a constant current power supply;
* How will I get heat out of the LED.

And then of course, you will look at the light color, or in our case, the light colors because we will be working with RGB lights. Most of what we say might be adapted for single-color LED or four-colors LEDs.

We like the LEDs from LedEngin and for today's purpose, we will use a [LZ4-00MC00 RGB LED]. We highly recommend that you purchase the one that is already soldered onto a small thermal PCB that will help with heat dissipation. You can [find it on mouser.com][mouser-lz4] for $15.

This LED (like all multi-colors LEDs) is actually several LED dice on one chip. If you take a look at the [datasheet][lz4-datasheet], you will see that the maximum forward voltage for the red, green, and blue dices are 2.96V, 8.32V, 4.48V (the forward voltage for green is actually much higher because there are two green dices in series). We also learn from the datasheet that the nominal forward current is 700 mA, therefore the total power of the LED is 2.96*0.7+8.32*0.7+4.48*0.7=11W.

That is pretty nice compared to our previous sub-watt LED or traditional 1W or 3W led!

Of course, you can pick from other manufacturers, Cree and OSRAM Semiconductors are famous LED manufacturers. Tell us about your experiments in the comments!

# Choosing a current source

The usual technique of putting a current limiting resistor on this type of LED will not work for two reasons. The first one is that the heat dissipated by this resistor would be huge (`P = I * U = 700mA * (Vin - Vf)` = a few watts) but mostly the forward voltage will decrease as the LED heats up and therefore the voltage on the series resistor will be increased which will in turn increase the intensity of the current and the light will not be stable at all. 

You could go on google and purchase a current source but they are usually kind of big and very expensive ($20). Knowing that we need three of them, we ruled this solution out.

So the best solution is to build your own, and we will see how to build one for a couple of dollars.

# A simple linear current source

The schema we introduce here uses a mosfet transistor that is controlled by a feedback mechanism through a current-sense resistor. It is heavily inspired by [Dan Goldwater][dangoldwater] ([MonkeyLectric][monkeylectric]) work posted on [Instructables][instructables].

![Linear current source](http://www.tbideas.com/blog/wp-content/uploads/2012/08/schema1.png)

The parts on this schematic are available for a few dollars. You can replace the BUZ71 by another MOSFET N-channel transistor in a TO220 package. Make sure it can switch 40V to be on the safe side.
For an RGB LED, we will build this three times: once for each channel.

## Let's discuss the schematics

When the PWM input (which will be driven by your micro-controller) is low, then Q1 is not conducting and nothing happens. The LED does not get any current and does not light up.

Things get interesting when you raise the PWM input HIGH (+5V), at this point current will start to flow through R1 and the gate of Q1 and it will let current flow from the source to the drain, therefore turning on the LED.

As current flows through the LED, the voltage drop of R2 increases (remember, U=RI) and when it reaches 0.5V (the forward junction drop of T1),  T1 will start conducting and current from R1 will flow through T1 instead of flowing through Q1, which will reduce the current going through Q1 and the LED.
This circuit will stabilize so that the voltage on R2 is the forward junction drop of T1 and that is how we can control the current going through the LED.

This is a very simple but very efficient schematics that you can use to power any type of LED.

## Setting the current through the LED

One of the crucial component on this schematic is R2. In our example R2 is 0.82 ohm which means that the current in R2 and the LED will reach 610mA (0.5V / 0.82ohm) before it starts turning on T1.
For 610mA, the power dissipated by R2 will be around 0.4W make sure to get a 1/2W or 1W resistor!

Depending on the maximum current you want in your LED, you will adapt the value of R2.

## Heat

If you look at voltage in the circuit, you can easily see that:

    V+ = Vled + Vq1 + Vr1

In this formula, we know V+, Vled and Vr1. V+ is your input voltage, Vled is the forward drop of the LED and Vr1 is 0.5V, the voltage needed to start turning on T1. So let us rewrite the formula as:

    Vq1 = V+ - Vled - Vr1

With an input voltage of 12V and a forward drop of 9V, we will have:

    Vq1 = 12 - 9 - 0.5 = 2.5V
    
We also know that the current going through the circuit is 610mA, therefore the power dissipated in Q1 is:

    Pq1 = Vq1 * Iq1 = 1.5W

## Best case scenario

From the formula above, we can deduce that the circuit is very efficient when Vq1 is equal to 0V, that is when V+ is equal to the forward voltage drop of the LED plus 0.5V. Anything above that on the input will be dissipated as heat in Q1.

Do note also that as things heat up, the forward voltage of the led will reduce a little bit, as well as T1 junction forward voltage (a few hundreds of milli-volts).

## Powering RGB Leds

Let's get back to the value of our LED. We have seen that the forward voltage is not the same for all colors. But you will very likely have only one power supply providing one common voltage to all the LEDs. Therefore there will be some heat dissipation and the bigger the difference in forward voltage, the bigger will be the heat dissipation.

For the LED we suggested above, you can choose to not put both green LEDs in series and power only one, this will reduce the voltage drop difference between red, green and blue and will avoid wasting too much power in the red and blue current sources.

# Build your Arduino high power RGB led shield

So now that we have a schematics that works (I hope you have tried it on your experimentation boards!), you can turn that into an Arduino shield with just a little effort. 

Here is an exemple of what it might look like on a pre-etched PCB:

![Arduino High power LED shield](http://www.tbideas.com/blog/wp-content/uploads/2012/08/illumi-v0.1.png)

Note that I have added heatsinks (and thermal paste) to the mosfet transistors. Even a few watts of power dissipated will heat the transistor pretty fast. Without heatsinks, they will not last long.

# More power!

If you are using higher power LEDs (or just more LEDs in series), the current will probably be the same but the voltage drop will be higher (that is because high power LEDs are actually more LEDs in series). Therefore the difference in drop between the different color will be even bigger and you will not be able to power them all from the same power supply. You have reached the limit of our linear current source.

The next step is to use a switching power supply that will be much more efficient, whatever the input voltage is. Our [design for Illumi v0.2][illumi-driver-v0.2] provides exactly that!


[previous-post]: http://www.tbideas.com/blog/illumi-101-iphone-controlled-rgb-light
[datasheet]: https://www.sparkfun.com/datasheets/Components/YSL-R596CR3G4B5C-C10.pdf
[lumen]: http://en.wikipedia.org/wiki/Lumen_(unit)
[lzp]: http://www.ledengin.com/products/emitters#LZP
[illumi]: http://www.tbideas.com
[mouser-lz4]: http://www.mouser.com/ProductDetail/LedEngin/LZ4-20MC00/?qs=%2fha2pyFaduhdcPvr3jm8Z%252b1vMoky8MR1gALPbizXkCL%2fpJr5JZov8w%3d%3d
[lz4-datasheet]: http://www.ledengin.com/files/products/LZ4/LZ4-00MC00.pdf
[instructables]: http://www.instructables.com/id/Circuits-for-using-High-Power-LED-s/step8/a-little-micro-makes-all-the-difference/
[dangoldwater]: http://www.instructables.com/member/dan/
[monkeylectric]: http://www.monkeylectric.com/
[illumi-driver-v0.2]: http://www.tbideas.com/blog/design-of-the-led-driver/
