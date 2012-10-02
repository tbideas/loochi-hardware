/*
 * pwm.c
 * 
 * Library to use PWM on AtTiny861
 * 
 * Copyright 2012 - Thomas Sarlandie - TBIdeas
 * Distributed under the CC-BY-SA license.
 * http://creativecommons.org/licenses/by-sa/3.0/
 */

#include <avr/io.h>
#include <util/delay.h>
#include "pwm.h"

/*
 * Initializes the PLL and the register for Fast PWM output.
 * 
 * The timer divider will be 2^(prescaler - 1). 
 * Set it to 0 to stop the timer. Max is 0x0F
 */
// FIXME: Only enabling 1B right now.
void pwm_fast_init(uint8_t prescaler)
{
	// 0010 0001
	//   ++      COM1B = 00 Clear OC1B on compare match 
	//         + PWM1B =  0 Enable PWM for 1B
	TCCR1A = 0x21;
	// 0000 0010
	//      ++++ CS3210 = 0001 Timer/Counter1 running with DIV/2
	TCCR1B = 0x00 | (0x0f & prescaler);
	
	// TCCR1C
	// Not used now (for Output D)

	// TCCR1D
	// 0000 0000
	//        ++ WGM11:10 = 00 - Fast PWM
	TCCR1D = 0x00;
	
	// PLLCSR
	// 0000 0010
	//        +  PLLE - Enable PLL
	PLLCSR = 0x02;
	
	_delay_ms(2); // Wait for the PLL to lock
	
	// Set timer1 source to be the PLL
	PLLCSR = 0x06;
}

/* Set the Ton period */
void pwm_set_on_b(uint8_t t)
{
	OCR1B = t;
}

/* Set the Ton + Toff period */
void pwm_set_period(uint8_t t)
{
	OCR1C = t;
}

	
