#ifndef __BSP_LED_H
#define __BSP_LED_H	 

#include "delay.h"


void InitLED(void);//≥ı ºªØ
void ToggleLED0(void);

#define LED0 PBout(14)		

#endif
