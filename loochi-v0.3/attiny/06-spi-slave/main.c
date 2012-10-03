#include <avr/io.h>
#include <avr/interrupt.h>


uint8_t c = 0;

/* Called when a byte is received by the USI */
ISR(SIG_USI_OVERFLOW)
{
	USIDR = c += USIBR;

	// Clear the interrupt
	USISR = (1 << USIOIF);
}


int main(void)
{
	// Status output
	DDRA |= _BV(PA7); 

	// Configure USI as SPI-Slave
	USICR = (1 << USIWM0) | (1 << USICS1) | (1 << USIOIE); // 3-wire mode + External positive edges + interrupts
	DDRA |= (1 << PA1); // Set DO as an output
	USIPP = 1; // We use PORTA for communication
	
	// Prepare the data to be returned
	USIDR = 0x42;

	// Get interrupts.
	sei();
	
	uint8_t i = 0;
	while (1) {
		if (i++ % 2 == 0) {
			PORTA |= _BV(PA7);
		}
		else {
			PORTA &= ~_BV(PA7);
		}
	}
}
