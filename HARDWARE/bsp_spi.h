#ifndef __BSP_SPI_H
#define __BSP_SPI_H

#include "gd32f4xx.h"
#include "sysconfig.h"

#define SPI2_CS  PDout(0)

void SpiInit(void);
uint8_t SpiReadWriteByte(uint8_t data);

#endif


