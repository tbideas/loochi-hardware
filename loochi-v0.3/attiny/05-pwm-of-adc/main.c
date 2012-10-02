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

// Brightness PWM
#define BPWM_MIN 0x0000
#define BPWM_MAX 0x01FF

uint16_t pwm_t = 0x0000;
// KO: uint16_t pwm_c = 0x0000;
// OK:
uint16_t pwm_c = 0x0000;

/*
 * Timer0 compare interrupt for our software PWM.
 * 
 * Timer0 runs at 8Mhz (1/8Mhz = 125nS) and hits compare every 0xFF (so every 32us).
 *
 * With 8 bit resolution, the total period of the signal is:
 *  T = (32us + x) * MaxValue 
 * Where x is the time needed to execute the interrupt (approximately 9us in 2012/10/01)
 *       MaxValue is BPWM_MAX
 * 
 * With BPWM_MAX = 0x0FF => 10ms ()
 * With BPWM_MAX = 0x1FF => 20ms (47Hz)
 * With BPWM_MAX = 0x3FF => 41ms ()
 *
 * Note: You cant use Timer0 overflow for this because you will be called 
 * every 8uS and the simple software PWM here takes about 9us to run...
 * Note: We could try running faster by reducing the compare value OCR0A
 * (this is why we are not using SIG_OVERFLOW0 now although it might be better because
 *  it would take priority over the ADC interrupt).
 */
ISR(SIG_OUTPUT_COMPARE0A)
{
	pwm_c++;
	if (pwm_c > BPWM_MAX) {
		pwm_c = 0;
	}
	
	if (pwm_c < pwm_t) {
		//PORTB |= 1;
		TCCR1A |= (1 << PWM1B);

		// If we are on and the ADC is available, start a conversion
//		if ((ADCSRA & (1 << ADSC)) == 0) {
			// Wait long enough for the RC network to charge
		if (pwm_c == 0x10) {
			ADCSRA |= (1 << ADSC);
		}
		
	}
	else {
		//PORTB &= ~1;
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
	
	// Adjust the output when we get the result of a conversion
	// we first test the adc value because when it is too low, dividing does not give good results
	if (adc < 0x200 && k < 275) {
		if (ton < 0xFF) {
			OCR1B = ++ton;
		}
	}
	else {
		if (ton > 1) {
			OCR1B = --ton;
		}
	}
}

int main(void)
{
	DDRB |= _BV(PB3);
	DDRB |= _BV(PB0); // raw-pwm output

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

	// Configure timer0
	TCCR0A = 0x00;
	TCCR0B = 0x01; // Enable counter without prescaling
	OCR0A = 0xFF; // compare at 0xFF
	TIMSK |= (1 << OCIE0A); // enable interrupt for compare0A
	
	sei();
	
	while(1) {
		if (timer % 100 == 0) {
			if ((PINA & (1 << B_UP)) == 0 && pwm_t < BPWM_MAX) {
				pwm_t++;
			}
			if ((PINA & (1 << B_DOWN)) == 0 && pwm_t > BPWM_MIN) {
				pwm_t--;
			}
		}
		
		if (timer++ == 50000 / LOOP_DELAY) {
			spi_master_write(pwm_t >> 8);
			spi_master_write(pwm_t);
			spi_master_write(ton);
			spi_master_write((adc & 0xFF00) >> 8);
			spi_master_write(adc & 0x00FF);

			timer = 0;
		}
		_delay_us(LOOP_DELAY);
	}
}
