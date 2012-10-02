/*
 * usi.c
 * 
 * Library to use the USI on AtTiny861
 * 
 * Copyright 2012 - Thomas Sarlandie - TBIdeas
 * Distributed under the CC-BY-SA license.
 * http://creativecommons.org/licenses/by-sa/3.0/
 */

#include <avr/io.h>
#include "usi.h"

void spi_master_init(uint8_t port)
{
	if (port == USI_PORTB) 
	{
		DDRB |= (1 << PB1) | (1 << PB2);
	}
	else if (port == USI_PORTA) 
	{
		DDRA |= (1 << PA1) | (1 << PA2);
		USIPP = 1;
	}
}

uint8_t spi_master_write(uint8_t byte)
{
	USIDR = byte;
	USISR = 1<<USIOIF; // clear counter overflow flag
 
	// tick the clock until the overflow is reached again
  while ( (USISR & _BV(USIOIF)) == 0 ) {
   	USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK)|(1<<USITC);
  }
	// return the value sent by the slave
  return USIDR;
}
