/*
 * adc.h
 * 
 * Library to use the ADC on AtTiny861
 * 
 * Copyright 2012 - Thomas Sarlandie - TBIdeas
 * Distributed under the CC-BY-SA license.
 * http://creativecommons.org/licenses/by-sa/3.0/
 */

#ifndef __ADC_H__
#define __ADC_H__

#include <inttypes.h>

#define VREF_VCC 0
#define VREF_AREF 1
#define VREF_I1_1 2
#define VREF_I2_56 6
#define VREF_I2_56_WITHCAP 7

void adc_init(uint8_t vref, uint8_t adc, uint8_t prescaler);
uint16_t adc_read();

#endif
