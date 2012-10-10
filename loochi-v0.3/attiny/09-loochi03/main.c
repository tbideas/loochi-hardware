

#include <avr/interrupt.h>
#include "globals.h"
#include "adc.h"
#include "brightness.h"
#include "current.h"
#include "serial.h"

#define LOOP_DELAY 100

void init_timer0(void);

/* Interrupt vectors */

// Timer0 overflow
ISR(SIG_OVERFLOW0)
{
	brightness_pwm_loop();
	adc_loop();
	serial_tick();
}

// ADC reading ready
ISR(SIG_ADC)
{
	/* Note: it's important to read ADCL first and then ADCH */
	uint16_t adc = ADCL;
	adc |= ADCH << 8;

	process_adc_reading(adc);
}

// Serial buffer overflow (1 byte received)
ISR(SIG_USI_OVERFLOW)
{
	serial_rx_byte(USIDR);
	
	// Clear the interrupt
	USISR = (1 << USIOIF);
}

int main(void)
{
	/* Use the hardware (Timer1) to generate a fast (125kHz) pwm
	 * that will drive the buck converter on/off.
	 */
	init_current_loop();
	
	/* Initializes the ADC */
	init_adc();
	
	/* Run a much slower (122Hz) PWM (based on Timer0 interrupts) */
	init_timer0();
	
	/* Initializes the serial port and prepare to receive data */
	init_serial();

	/* Initialize global variables */
	redcpwm = 0x20;
	greencpwm = 0x00;
	bluecpwm= 0xFf; /* inverted */

	pwm_c = 0x00;
	pwm_red =   0x0FF;
	pwm_green = 0x000;
	pwm_blue =  0x000;

	/* Enable interrupts and let the show begin! */
	sei();
	
	while(1) {}
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
