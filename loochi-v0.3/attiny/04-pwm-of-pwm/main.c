#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usi.h"
#include "pwm.h"

#define B_UP 6
#define B_DOWN 7

#define LOOP_DELAY 50

uint8_t spi_transfer(uint8_t data);

uint8_t pwm_t = 0x80;
uint8_t pwm_c = 0x00;

ISR(SIG_OVERFLOW1)
{
	pwm_c++;
	if (pwm_c < pwm_t) {
		PORTB |= 1;
		TCCR1A |= (1 << PWM1B);
	}
	else {
		PORTB &= ~1;
		TCCR1A &= ~(1 << PWM1B);
	}
}

int main()
{
	DDRB |= _BV(PB3);  // PB3 is the TMR1-B-Output
	DDRB |= _BV(PB0); // test

	TIMSK |= 1 << TOIE1; // enable interrupt for timer1 overflow
	
	pwm_fast_init(2);
	spi_master_init(USI_PORTA);
	
	// Enable pullups on button inputs
	PORTA |= 1 << B_DOWN;
	PORTA |= 1 << B_UP; 

	uint8_t ton = 0x40;

	uint8_t timer = 0;
	
	pwm_set_period(0xFF);
	pwm_set_on_b(ton);

	// enable interrupts
	sei();

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
