#ifndef __GLOBALS_H__
#define __GLOBALS_H__

/* Current control PWM min/max and global variables */
#define CPWM_MIN 0x01
#define CPWM_MAX 0xC0

#define redcpwm    		OCR1A
#define greencpwm  		OCR1B
#define bluecpwm   		OCR1D

extern uint16_t redadc, greenadc, blueadc;

#endif