#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "globals.h"
#include "brightness.h"

uint16_t pwm_c;
uint16_t pwm_red;
uint16_t pwm_green;
uint16_t pwm_blue;

/*
 * Called every 32us to do a software PWM of the fast-pwm outputs.
 * In ascii art: ||||__________ ||||_______
 * where ||| is the fast pwm and ___ is when the fast pwm is muted bu the slower PWM.
 *
 * Using this double-PWM technique, we can have a constant current running through the LED
 * and still control their brightness.
 *
 * Depending on the resolution MaxValue, the total period of the signal is:
 *  T = (32us + x) * MaxValue 
 * Where x is the time needed to execute the interrupt (approximately 9us in 2012/10/01)
 *       MaxValue is BPWM_MAX
 * 
 * With BPWM_MAX = 0x0FF => 10ms (100Hz) <---- Measured: 
 * With BPWM_MAX = 0x1FF => 20ms (47Hz)  <---- Measured: 15.8ms period for 63.29 Hz
 * With BPWM_MAX = 0x3FF => 41ms ()
 *
 * Note: You cant use Timer0 overflow for this because you will be called 
 * every 8uS and the simple software PWM here takes about 9us to run...
 */
void brightness_pwm_loop()
{
	// Increment our counter and reset when we reach the top.
	pwm_c++;
	if (pwm_c > BPWM_MAX) {
		pwm_c = 0;
		
		// Activate the output if pwm > 0 when we start a new loop
		if (pwm_red > 0)
			TCCR1A |= (1 << PWM1A);
		if (pwm_green > 0)
		 	TCCR1A |= (1 << PWM1B);
		if (pwm_blue > 0)
			TCCR1C |= (1 << PWM1D);
	}
//	else {
		/* Turn red off */
		if (pwm_c == pwm_red) {
			TCCR1A &= ~(1 << PWM1A);
		}
		/* Turn green off */
		if (pwm_c == pwm_green) {
			TCCR1A &= ~(1 << PWM1B);
		}
		/* Turn blue off */
		if (pwm_c == pwm_blue) {
			TCCR1C &= ~(1 << PWM1D);
		}
	//}
}

