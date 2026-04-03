#ifndef __DELAY_H
#define __DELAY_H 			   
#include "sysconfig.h" 
#include "gd32f4xx.h"

	 
void delay_init(void);
void delay_us(uint32_t nus);
void delay_ms(uint16_t nms);

#endif





























