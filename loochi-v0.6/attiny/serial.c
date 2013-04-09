#include <avr/io.h>
#include <avr/interrupt.h>
#include "globals.h"
#include "serial.h"

/* Serial interface buffer */
#define USI_BUF_LEN 8
uint8_t usi_buffer[USI_BUF_LEN];
uint8_t usi_counter;

/*
 * Configure the Universal serial interface.
 */
void init_serial()
{
	/* Set Data-Output (PA1/DO) as an output */
	DDRA |= (1 << PA1);
	/* 3-wire mode + External positive edges + interrupts */
	USICR = (1 << USIWM0) | (1 << USICS1) | (1 << USIOIE);
	/* Use PORTA (instead of PORTB by default) */
	USIPP = 1; // We use PORTA for communication
	/* Clear the interrupt flag before we start */
	USISR = (1 << USIOIF);


  /* Enable pull-up on PB2 (Chip Select) */
	PORTB |= (1 << PB2);
	/* Enable Pin Change interrupt on PB2 change (PCINT10) */
  GIMSK = 0;
  GIMSK |= (1 << PCIE0);
  PCMSK1 = 0;
  PCMSK1 |= (1 << PCINT10);
  
  usi_counter = 0;
}

/* Called when the chip select is enabled/disabled to process a command */
void serial_cs(uint8_t cs)
{
  // Note: According to Wikipedia, CS is active low
  if (cs == 0) { /* Start transmission */
    usi_counter = 0;
  }
  else {
		pwm_red = usi_buffer[0];
		pwm_green = usi_buffer[1];
		pwm_blue = usi_buffer[2];
  }
}

/* Called when a byte is received by the USI */
void serial_rx_byte(uint8_t byte)
{
	usi_buffer[usi_counter++] = byte;
  if (usi_counter > USI_BUF_LEN)
    usi_counter = 0;
}
