#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

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
		if ((ADCSRA & (1 << ADSC)) == 0 && pwm_c > 0x10) {
			// Wait long enough for the RC network to charge
//		if (pwm_c == 0x26) {
			ADCSRA |= (1 << ADSC);
		}
		
	}
	else {
		//PORTB &= ~1;
		TCCR1A &= ~(1 << PWM1B);
	}
}

/* Expected value of ADC for a given value of Ton to get 700mA on the LED
 * 
 * The formula is: ADC = 2*Ton/Tperiod*Iled*1024/2.56
 * Everything is known except Ton.
 * $ perl -e 'print join(",", map { sprintf "%.0f", (2*$_/0xff*0.7*1024/2.56)} 0..255)'
 */
PROGMEM uint16_t ADC_TARGET_VALUES[] = { 
	0,2,4,7,9,11,13,15,18,20,22,24,26,29,31,33,35,37,40,42,44,46,48,51,53,55,57,59,61,64,66,68,70,72,75,77,79,81,83,86,88,90,92,94,97,99,101,103,105,
	108,110,112,114,116,119,121,123,125,127,130,132,134,136,138,141,143,145,147,149,152,154,156,158,160,163,165,167,169,171,173,176,178,180,182,184,
	187,189,191,193,195,198,200,202,204,206,209,211,213,215,217,220,222,224,226,228,231,233,235,237,239,242,244,246,248,250,253,255,257,259,261,264,
	266,268,270,272,275,277,279,281,283,285,288,290,292,294,296,299,301,303,305,307,310,312,314,316,318,321,323,325,327,329,332,334,336,338,340,343,
	345,347,349,351,354,356,358,360,362,365,367,369,371,373,376,378,380,382,384,387,389,391,393,395,397,400,402,404,406,408,411,413,415,417,419,422,
	424,426,428,430,433,435,437,439,441,444,446,448,450,452,455,457,459,461,463,466,468,470,472,474,477,479,481,483,485,488,490,492,494,496,499,501,
	503,505,507,509,512,514,516,518,520,523,525,527,529,531,534,536,538,540,542,545,547,549,551,553,556,558,560
};
#define ADC_TARGET(a) (pgm_read_word_near(ADC_TARGET_VALUES + a)) // CIE Lightness loopup table function

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

	if (adc < ADC_TARGET(ton) && ton < CPWM_MAX) {
		OCR1B = ++ton;
	}
	else if (ton > CPWM_MIN) {
			OCR1B = --ton;
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
