#include <avr/io.h>
#include <avr/interrupt.h>
#include "globals.h"
#include "serial.h"

/* Serial interface buffer */
uint8_t usi_buffer[8];
uint8_t usi_counter;
/* USI timer incremented by serial_tick() */
uint8_t usi_timeout;
#define USI_TIMEOUT_MAX 0xFF




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

	/* We will use this var to detect gap in the transmission which indicates
	 * the start of a new packet.
	 * The first byte received will be the first  of a new packet.
	 */
	usi_timeout = 0xFF;
}

/* Called regularly to increment the serial timeout */
void serial_tick()
{
	// Increment usi_timeout without overflow
	if (usi_timeout < USI_TIMEOUT_MAX)
		usi_timeout++;
}

/* Called when a byte is received by the USI */
void serial_rx_byte(uint8_t byte)
{
	/* usi_timeout is used to reset the byte counter and re-synchronize with sender.
	 * It is incremented every 3.2us by TimerO overflow. If we dont get any byte during
	 * 3.2us*255 (=8ms), we will consider that the next byte is the beginning of a new sentence. 
	 */
	if (usi_timeout >= USI_TIMEOUT_MAX) 
		usi_counter = 0;
		
	usi_buffer[usi_counter++] = byte;

	if (usi_counter == 3) {
		pwm_red = usi_buffer[0];
		pwm_green = usi_buffer[1];
		pwm_blue = usi_buffer[2];
		usi_counter = 0;
	}
	
	USIDR = usi_counter;

	// Always reset the counter when we get a byte
	usi_timeout = 0;
}
