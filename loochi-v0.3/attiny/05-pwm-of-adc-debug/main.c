#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "usi.h"
#include "pwm.h"
#include "adc.h"

#define B_UP 6
#define B_DOWN 7

#define LOOP_DELAY 100

// Current PWM
#define CPWM_MIN 0x01
#define CPWM_MAX 0xC0
uint8_t ton = CPWM_MIN;
uint32_t toto = 0;

// Brightness PWM
#define BPWM_MIN 0x0040
#define BPWM_MAX 0x0080
uint16_t pwm_t = 0x00FF;
// KO: uint16_t pwm_c = 0x0000;
// OK:
uint16_t pwm_c = 0x0000;

// This will be called every 8 uS (1/32Mhz * 256)
// Therefore the brightness PWM has a minimum period of 8uS
// and a maximum period of 2mS and a frequency of 488Hz.
ISR(SIG_OVERFLOW1)
{
	pwm_c++;
	if (pwm_c > BPWM_MAX) {
		pwm_c = 0;
	}
	
	if (pwm_c < pwm_t) {
		TCCR1A |= (1 << PWM1B);
	}
	else {
		TCCR1A &= ~(1 << PWM1B);
	}
}

/*
 * This will be called 104 uS after the conversion is started above
 * but the measure is taken a t0+8uS so we can effectively measure
 * even when the PWM is only at a value of 1*8uS.
 *
 * If the brightness is at 0x01 (worst case) and the correct pwm at 0xFF (worst case), it will
 * take 2ms * 0xFF = 510ms for the current to reach the correct value.
 *
 * convSpeed = max(1, brightness * 8uS / 104uS)
 *  -> We can convert convSpeed times per each brightness cycle
 * stabilizeTime = NumberOfConversionsNeeded / convSpeed * 2mS
 *  -> NumberOfConversionsNeeded = 0xFF if we start @0 but we could start higher
 *
 * With brightness 0x01: 510mS
 * With brightness 0x02: 510mS
 * With brightness 0x20: 255mS
 * With brightness 0x80: 56mS
 * With brightness 0xFF: 26ms
 * 
 */
uint16_t adc;

ISR(SIG_ADC)
{
	adc = ADCL;
	adc |= ADCH << 8;

	uint32_t k = (adc*100) / ton;
	
	OCR1B = ton = 0x80;
	/*
	// Adjust the output when we get the result of a conversion
	if (adc < 0x200 && k < 275) {
		if (ton < 0xFF) {
			OCR1B = ++ton;
		}
	}
	else {
		//if (ton > 0) {
		OCR1B = --ton;
		//}
	}*/
}

int main(void)
{
	DDRB |= _BV(PB3);
	DDRB |= _BV(PB0); // raw-pwm output
	TIMSK |= 1 << TOIE1; // enable interrupt for timer1 overflow

	spi_master_init(USI_PORTA);
	
	// Enable pullups on button inputs
	PORTA |= 1 << B_DOWN;
	PORTA |= 1 << B_UP; 

	uint16_t timer = 0;
	
	pwm_fast_init(2);
	pwm_set_period(0xFF);
	pwm_set_on_b(ton);

	DDRA |= _BV(PA0); // Force Data in (MISO) to 0
	
	// prescaler: 6 = div/64; fADC = 125kHz; 1 clockcycle = 8uS; 1 conversion=104uS
	adc_init(VREF_I2_56, 0x12, 6);

	sei();
	
	while(1) {
			// If ADC available, start a conversion
			if ((ADCSRA & (1 << ADSC)) == 0) {
				ADCSRA |= (1 << ADSC);
			}
		
		
			if ((PINA & (1 << B_UP)) == 0 && pwm_t < 0xff) {
				pwm_t++;
			}
			if ((PINA & (1 << B_DOWN)) == 0 && pwm_t > 0) {
				pwm_t--;
			}
		
		if (timer++ == 50000 / LOOP_DELAY) {
			spi_master_write(ADCSRA);
			spi_master_write(pwm_t);
			spi_master_write(ton);
			spi_master_write((adc & 0xFF00) >> 8);
			spi_master_write(adc & 0x00FF);

			timer = 0;
		}
		_delay_us(LOOP_DELAY);
	}
}
