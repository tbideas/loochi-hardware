#include <avr/io.h>
#include <avr/pgmspace.h>
#include "globals.h"
#include "adc.h"

/* ADC configuration for each sense resistor */
#define ADMUX_RED    	((1 << REFS1) | 0x12)
#define ADMUX_GREEN  	((1 << REFS1) | 0x17)
#define ADMUX_BLUE   	((1 << REFS1) | 0x19)
uint8_t adc_discard;

/*
 * Truthtable to choose which channel the ADC should read next.
 * Input: 6bits
 *  00XXXXXX
 *    ++     -> previous selected channel (00=red, 01=green, 10=blue)
 *      +    -> have we got a valid reading on this channel already
 *       +   -> is red on
 *        +  -> is green on
 *         + -> is blue on
 * Output: next value of ADMUX or 0 if no new ADC is to be chosen
 */
PROGMEM uint8_t ADC_TRUTHTABLE[] = {
	0,ADMUX_BLUE,ADMUX_GREEN,ADMUX_GREEN,ADMUX_RED,ADMUX_RED,ADMUX_RED,ADMUX_RED,0,ADMUX_BLUE,ADMUX_GREEN,ADMUX_GREEN,ADMUX_RED,
	ADMUX_BLUE,ADMUX_GREEN,ADMUX_GREEN,0,ADMUX_BLUE,ADMUX_GREEN,ADMUX_GREEN,ADMUX_RED,ADMUX_BLUE,ADMUX_GREEN,ADMUX_GREEN,0,
	ADMUX_BLUE,ADMUX_GREEN,ADMUX_BLUE,ADMUX_RED,ADMUX_BLUE,ADMUX_RED,ADMUX_BLUE,0,ADMUX_BLUE,ADMUX_GREEN,ADMUX_BLUE,ADMUX_RED,
	ADMUX_BLUE,ADMUX_RED,ADMUX_BLUE,0,ADMUX_BLUE,ADMUX_GREEN,ADMUX_GREEN,ADMUX_RED,ADMUX_RED,ADMUX_RED,ADMUX_RED
};

/* Expected reading of the ADC for a given value of Ton to get 700mA on the LED
 *
 * The formula is: ADC = 2*Ton/Tperiod*Iled*1024/2.56
 * Everything is known except Ton.
 * $ perl -e 'print join(",", map { sprintf "%.0f", (2*$_/0xff*0.7*1024/2.56)} 0..255)'
 * (the first value was manually hacked to -1 to make sure the thing starts)
 */
PROGMEM int16_t ADC_TARGET_VALUES[] = {
	-1,2,4,7,9,11,13,15,18,20,22,24,26,29,31,33,35,37,40,42,44,46,48,51,53,55,57,59,61,64,66,68,70,72,75,77,79,81,83,86,88,90,92,94,97,99,101,103,105,
	108,110,112,114,116,119,121,123,125,127,130,132,134,136,138,141,143,145,147,149,152,154,156,158,160,163,165,167,169,171,173,176,178,180,182,184,
	187,189,191,193,195,198,200,202,204,206,209,211,213,215,217,220,222,224,226,228,231,233,235,237,239,242,244,246,248,250,253,255,257,259,261,264,
	266,268,270,272,275,277,279,281,283,285,288,290,292,294,296,299,301,303,305,307,310,312,314,316,318,321,323,325,327,329,332,334,336,338,340,343,
	345,347,349,351,354,356,358,360,362,365,367,369,371,373,376,378,380,382,384,387,389,391,393,395,397,400,402,404,406,408,411,413,415,417,419,422,
	424,426,428,430,433,435,437,439,441,444,446,448,450,452,455,457,459,461,463,466,468,470,472,474,477,479,481,483,485,488,490,492,494,496,499,501,
	503,505,507,509,512,514,516,518,520,523,525,527,529,531,534,536,538,540,542,545,547,549,551,553,556,558,560
};
#define ADC_TARGET(a) (pgm_read_word_near(ADC_TARGET_VALUES + a))

/* Count the number of values that we have discarded.
 * Every time we switch from one channel to the other, we need to discard at least one
 * before trusting the reading.
 */
uint8_t adc_discard = 0;

/* Forward declaration */
uint8_t adc_choose_nextchannel(void);


/*
 * Configure the Analog-Digital Converter.
 */
void init_adc(void)
{
	/* Set the voltage reference to 2.56V and select a channel */
	ADMUX = ADMUX_RED;
	/* This bit needs to be set for the 2.56 Voltage reference */
	ADCSRB = (1 << REFS2);

	/* Set the prescaler to DIV64. The ADC clock will run @125kHz
	 * which is in the recommended range (50kHz to 200kHz).
	 * 1 ADC clockcycle will be 8uS.
	 * Sampling will be done 1.5 cycles * 8uS = 12uS after the start of a conversion.
	 * Result will be available 13 cycles * 8us = 104uS after the start of a conversion.
	 */
	ADCSRA = (1 << ADPS2) | (1 << ADPS1)
	/* Enable interrupts for ADC result */
		| (1 << ADIE)
	/* Enable the ADC */
		| (1 << ADEN);
}

/*
 * This function checks to see if the ADS is available and starts a new conversion on the right channel.
 */
void adc_loop(void)
{
	/*
	 * If the ADC is available, and if the signals have been on for at least a few cycles,
	 * then we can choose one channel and run a conversion.
	 */
	if ((ADCSRA & (1 << ADSC)) == 0 && pwm_c > 0x10) {
		/* Choosing the right channel is actually pretty complicated so we use a truth table */
		uint8_t next_channel = adc_choose_nextchannel();

		if (next_channel == ADMUX) {
			/* We stay on the same ADC channel - Just restart a new conversion */
			ADCSRA |= (1 << ADSC);
		}
		else if (next_channel != 0) {
			/* Change of ADC channel. We will need to discard. */
			adc_discard = 0;
			ADMUX = next_channel;
		}
	}
}

/*
 * Calculate the input to the truth table (read description above).
 *
 * Returns 0 if no ADC should be chosen or the ADMUX value to use.
 */
uint8_t adc_choose_nextchannel(void)
{
	uint8_t truth;
	if (ADMUX == ADMUX_RED)		truth = 0x00;
	if (ADMUX == ADMUX_GREEN)	truth = 0x10;
	if (ADMUX == ADMUX_BLUE)	truth = 0x20;

	if (adc_discard > 1)
		truth |= 0x8;
	if (pwm_c < pwm_red)
		truth |= 0x4;
	if (pwm_c < pwm_green)
		truth |= 0x2;
	if (pwm_c < pwm_blue)
		truth |= 0x1;

	return pgm_read_word_near(ADC_TRUTHTABLE + truth);
}

/*
 * This function gets and ADC reading and adjust the current PWM to get the current we want.
 *
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
void process_adc_reading(uint16_t adc)
{
	/* We always need to discard one measure */
	if (adc_discard++ < 1)
		return;

	if (ADMUX == ADMUX_RED) {
		if (adc < ADC_TARGET(redcpwm) && redcpwm < CPWM_MAX) {
			redcpwm++;
		}
		else if (redcpwm > CPWM_MIN) {
			redcpwm--;
		}
	}
	if (ADMUX == ADMUX_GREEN) {
		if (adc < ADC_TARGET(greencpwm) && greencpwm < CPWM_MAX) {
			greencpwm++;
		}
		else if (greencpwm > CPWM_MIN) {
			greencpwm--;
		}
	}
	if (ADMUX == ADMUX_BLUE) {
		/* Remember that the blue output is inverted */
		if (adc < ADC_TARGET(0xFF - bluecpwm) && bluecpwm > CPWM_MIN) {
			bluecpwm--;
		}
		else if (bluecpwm < CPWM_MAX) {
			bluecpwm++;
		}
	}
}

