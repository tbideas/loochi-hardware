/*
 * pwm.h
 * 
 * Library to use PWM on AtTiny861
 * 
 * Copyright 2012 - Thomas Sarlandie - TBIdeas
 * Distributed under the CC-BY-SA license.
 * http://creativecommons.org/licenses/by-sa/3.0/
 */

#ifndef __PWM_H__
#define __PWM_H__

#include <inttypes.h>

void pwm_fast_init(uint8_t prescaler);
void pwm_set_on_b(uint8_t t);
void pwm_set_period(uint8_t t);

#endif