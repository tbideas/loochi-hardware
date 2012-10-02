#include <avr/io.h>
#include <util/delay.h>
#include "usi.h"
#include "pwm.h"
#include "adc.h"

#define B_UP 6
#define B_DOWN 7

#define LOOP_DELAY 1

int main()
{
	DDRB |= _BV(PB3);
	
	spi_master_init(USI_PORTA);
	
	// Enable pullups on button inputs
	PORTA |= 1 << B_DOWN;
	PORTA |= 1 << B_UP; 

	uint8_t ton = 0x40;
	uint16_t timer = 0;
	
	pwm_fast_init(2);
	pwm_set_period(0xFF);
	pwm_set_on_b(ton);

	DDRA |= _BV(PA0); // Force Data in (MISO) to 0
	
	// prescaler: 6 = div/64 = 1 clockcycle = 8uS ; 1 conversion=104uS
	adc_init(VREF_I2_56, 0x12, 6);
	
	while(1) {
		/*
		if ((PINA & (1 << B_UP)) == 0) {
			pwm_set_on_b(ton++);
		}
		if ((PINA & (1 << B_DOWN)) == 0) {
			pwm_set_on_b(ton--);
		}
		*/

		uint16_t adc = adc_read();
		
		if (adc * 100 / ton < 275) {
			if (ton < 0xFF) {
				pwm_set_on_b(++ton);
			}
		}
		else {
			if (ton > 1) {
				pwm_set_on_b(--ton);
			}
		}
		
		if (timer++ == 1000 / LOOP_DELAY) {
			spi_master_write(ton);
			spi_master_write((adc & 0xFF00) >> 8);
			spi_master_write(adc & 0x00FF);

			timer = 0;
		}
		_delay_ms(LOOP_DELAY);
	}
}
