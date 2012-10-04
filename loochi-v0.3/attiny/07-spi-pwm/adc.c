/*
 * adc.c
 * 
 * Library to use the ADC on AtTiny861
 * 
 * Copyright 2012 - Thomas Sarlandie - TBIdeas
 * Distributed under the CC-BY-SA license.
 * http://creativecommons.org/licenses/by-sa/3.0/
 */

#include <avr/io.h>
#include "adc.h"

/*
 * vref: source of voltage reference (VREF_*)
 * adc:  which ADC to use (single-ended between 0 and 10 or check datasheet)
 * prescaler: 3bits, check datasheet page 159 for actual effect
 */
void adc_init(uint8_t vref, uint8_t adc, uint8_t prescaler)
{
	// Initialize ADC registers
	ADMUX = (vref & 0x03) << 6;
	ADMUX |= adc & 0x1F;
	ADCSRA = prescaler & 0x07;
	
	ADCSRB = 0;
	if (vref & 0x04) 
		ADCSRB |= 1 << REFS2;
	if (adc & 0x20)
		ADCSRB |= 1 << MUX5;
	
	// Enable interrupts
	ADCSRA |= (1 << ADIE);

	// Enable and start a conversion
	ADCSRA |= (1 << ADEN);
}
