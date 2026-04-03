#ifndef __WS25QXX_DRV_H
#define __WS25QXX_DRV_H

#include "bsp_spi.h"


#define W25Q_CMD_WRITE_ENABLE               0x06
#define W25Q_CMD_READ_STATUS1               0x05
#define W25Q_CMD_JEDEC_ID                   0x9F
#define W25Q_CMD_MANUFACT_DEVICE_ID         0x90
#define W25Q_CMD_READ_DATA                  0x03
#define W25Q_CMD_PAGE_PROGRAM               0x02
#define W25Q_CMD_SECTOR_ERASE               0x20


void W25QXXInit(void);
void FlashTest();
void W25QXX_Read(uint32_t Address, uint8_t *Buffer, uint32_t Length);
void W25QXX_Write(uint32_t WriteAddr,uint8_t* pBuffer,uint16_t NumByteToWrite);
void W25QXX_SectorErase(uint32_t Address);
void W25QXX_WriteBuffer(uint32_t Address, uint8_t *Buffer, uint32_t Length);
uint8_t W25QXX_IsReady(void);
void W25QXX_Write_NoCheck(u32 WriteAddr,u8* pBuffer,u16 NumByteToWrite);



#endif
