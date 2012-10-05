#ifndef __GLOBALS_H__
#define __GLOBALS_H__

/* Brightness PWM min/max and global variables */
#define BPWM_MIN 0x0000
#define BPWM_MAX 0x01FF

extern uint16_t pwm_c;
extern uint16_t pwm_red;
extern uint16_t pwm_green;
extern uint16_t pwm_blue;

/* Current control PWM min/max and global variables */
#define CPWM_MIN 0x01
#define CPWM_MAX 0xC0

/* WARNING - for test on the prototype they are inverted! (also in brightness.c)*/
#define redcpwm    		OCR1B
#define greencpwm  		OCR1A
#define bluecpwm   		OCR1D

#endif