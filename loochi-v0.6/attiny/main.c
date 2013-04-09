
#include <util/delay.h>
#include <avr/interrupt.h>
#include "globals.h"
#include "adc.h"
#include "brightness.h"
#include "current.h"
#include "serial.h"

#define LOOP_DELAY 100

void init_timer0(void);

/* 
 * Interrupt vectors
 * (sorted by priority) 
 */

// Pin change interrupt (CS changed)
ISR(SIG_PIN_CHANGE)
{
  serial_cs(PINB & (1 << PB2));
}

// Timer0 overflow - Every 32us
ISR(SIG_OVERFLOW0)
{
	brightness_pwm_loop();
	adc_loop();
}

// Serial buffer overflow (1 byte received)
ISR(SIG_USI_OVERFLOW)
{
	serial_rx_byte(USIDR);
	
	// Clear the interrupt
	USISR = (1 << USIOIF);
}

// ADC reading ready
ISR(SIG_ADC)
{
	/* Note: it's important to read ADCL first and then ADCH */
	uint16_t adc = ADCL;
	adc |= ADCH << 8;

	process_adc_reading(adc);
}

int main(void)
{
	/* Use the hardware (Timer1) to generate a fast (250kHz) pwm
	 * that will drive the buck converter on/off.
	 * 
	 * Initialize the PWM value *before*.
	 */
	redcpwm = 0x00;
	greencpwm = 0x00;
	bluecpwm= 0xFF; /* inverted */
	init_current_loop();
	
	/* Initializes the ADC */
	init_adc();
	
	/* Run a much slower (122Hz) PWM (based on Timer0 interrupts) */
	init_timer0();
	
	/* Initializes the serial port and prepare to receive data */
	init_serial();

	/* Initialize global variables */
	pwm_c = 0x00;
	pwm_red =   0x000;
	pwm_green = 0x000;
	pwm_blue =  0x000;

	/* Enable pull-up on PB0 */
	PORTB |= (1 << PB0);

	/* Enable interrupts and let the show begin! */
	sei();

	while(1) {
		_delay_ms(10);
		// If button pressed
		if ((PINB & (1 << PB0)) == 0) {
			if (pwm_red == 0 && pwm_green == 0 && pwm_blue == 0) {
				pwm_red = pwm_green = pwm_blue = 0xFF;
				
				_delay_ms(30);
			}
			else {
				pwm_red = pwm_green = pwm_blue = 0;
				_delay_ms(30);
			}
		}
	}
}

/*
 * Timer0 is used to generate software interrupts that we use for different purpose.
 * 
 * This counter counts @8Mhz and will overflow every 256 ticks so the interrupt will be
 * called every: t = 1/8Mhz * 256 = 32uS
 */
void init_timer0()
{
	/* Enable counter with no prescaling (speed will be 8Mhz) */
	TCCR0B = (1 << CS00);
	
	/* Enable interrupt when an overflow occurs */
	TIMSK |= (1 << TOIE0);
}
