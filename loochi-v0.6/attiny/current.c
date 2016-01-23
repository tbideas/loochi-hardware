#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "current.h"

/*
 * Timer1 is used to generate three different PWM signals that will let
 * current flow through the LED and the inductor.
 * When the signal is On, the current could be infinite but the inductor will limit that
 * and the LED will receive a progressively increasing current.
 * When the signal is Off, the current could be 0 but the inductor will resist that sudden
 * change and will receive a progressively decreasing current.
 *
 * By changing the period of the PWM signals, we can control the average current flowing
 * through the LED.
 * 
 * The period of the PWM signals is defined by the speed of Timer1 clock (64Mhz here) multiplied
 * by OCR1C (counter TCNT1 is reset when it reachs OCR1C).
 *  With OCR1C default value 0xFF, the period is: t = 256 * 1/64M = 4us
 * 
 * The period of the Red, Green and Blue PWM are defined by the values in OCR1A, OCR1B, OCR1D.
 * Red => OC1A/PB1 Green => OC1B/PB3 Blue => -OC1D/PB4
 * Note: The blue output is inverted (for pin-space reason we could not use the normal output).
 */
void init_current_loop(void)
{
	/* Set direction of PWM output pins */
	DDRB = _BV(REDPIN) | _BV(GREENPIN) | _BV(BLUEPIN);

	/* Enable PWM output for OCR1A and OCR1B */
	TCCR1A = (1 << PWM1A) | (1 << PWM1B);
	
	/* Set Timer1 prescaler to 1 - The frequency will be 64Mhz */
	TCCR1B = (1 << CS10);

	/* Set COM1A = COM1B = 0b10 => Clear on compare match */
	TCCR1C = (1 << COM1A1S) | (1 << COM1B1S)
	/* COM1D = 0b01 => Enable inverted output OCR1D and clear on compare match */
		| (1 << COM1D0)
	/* Enable PWM output for OCR1D */
		| (1 << PWM1D);

	/* Enable the PLL */
	PLLCSR = (1 << PLLE);
	
	/* Wait for the PLL to lock */
	_delay_ms(2);
	
	/* Set the PLL as clock source of Timer1 */
	PLLCSR = 0x06;
}

