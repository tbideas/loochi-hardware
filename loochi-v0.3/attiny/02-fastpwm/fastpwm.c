#include <avr/io.h>
#include <util/delay.h>
#include "usi.h"
#include "pwm.h"

#define B_UP 6
#define B_DOWN 7

#define LOOP_DELAY 50

uint8_t spi_transfer(uint8_t data);

int main()
{
	DDRB |= _BV(PB3);
	
	pwm_fast_init(2);
	spi_master_init(USI_PORTA);
	
	// Enable pullups on button inputs
	PORTA |= 1 << B_DOWN;
	PORTA |= 1 << B_UP; 

	uint8_t ton = 0x40;

	uint8_t timer = 0;
	
	pwm_set_period(0xFF);
	pwm_set_on_b(ton);

	while(1) {
		if ((PINA & (1 << B_UP)) == 0) {
			pwm_set_on_b(ton++);
		}
		if ((PINA & (1 << B_DOWN)) == 0) {
			pwm_set_on_b(ton--);
		}
		
		if (timer++ == 1000 / LOOP_DELAY) {
			spi_master_write(ton);
			timer = 0;
		}
		_delay_ms(50);
	}
}
