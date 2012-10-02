/*
 * usi.h
 * 
 * Library to use the USI on AtTiny861
 * 
 * Copyright 2012 - Thomas Sarlandie - TBIdeas
 * Distributed under the CC-BY-SA license.
 * http://creativecommons.org/licenses/by-sa/3.0/
 */

#ifndef __USI_H__
#define __USI_H__

#include <inttypes.h>

#define USI_PORTB 0
#define USI_PORTA 1

void spi_master_init(uint8_t port);
uint8_t spi_master_write(uint8_t byte);

#endif