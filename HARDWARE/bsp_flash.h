#ifndef __BSP_FLASH_H
#define __BSP_FLASH_H

#include "gd32f4xx.h"

void Flash_IF_Init(void);
void Flash_IF_Finish(void);
int Flash_IF_App_Erase(int StartSectorId, int EndSectorId);
int Flash_IF_Write(__IO uint32_t *address, uint32_t *buffer, int length);


#endif
