/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Basic definitions of FatFs */
#include "diskio.h"		/* Declarations FatFs MAI */

/* Example: Declarations of the platform and disk functions in the project */
#include "w25qxx_drv.h"
#include "string.h"
#include "bsp_usart.h"

/* Example: Mapping of physical drive number for each drive */
#define DEV_W25Q	0	/* Map FTL to physical drive 0 */
#define DEV_MMC		1	/* Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Map USB MSD to physical drive 2 */

#define FLASH_SECTOR_SIZE   512      // FATFS逻辑扇区大小
#define FLASH_SECTOR_COUNT  2048*5  // W25Q128 = 16MB，按需分配 5MB给FATFS
#define FLASH_BLOCK_SIZE    8        // 擦除块大小(4096/512=8个逻辑扇区)
#define FLASH_ERASE_SIZE    4096     // Flash物理擦除大小

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv
)
{
	if (pdrv != 0)
		return STA_NODISK;

	if (W25QXX_IsReady())
		return 0;              // OK

	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

static uint8_t w25_inited = 0;

DSTATUS disk_initialize(BYTE pdrv)
{
    if(pdrv == DEV_W25Q)
    {
        if(!w25_inited)
        {
            W25QXXInit();
            w25_inited = 1;
            LogPrintf("W25QXX init done\r\n");
        }
        return 0;
    }
    return STA_NOINIT;
}


/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
	for(;count>0;count--)
	{
		W25QXX_Read(sector*FLASH_SECTOR_SIZE,buff,FLASH_SECTOR_SIZE);
		sector++;
		buff+=FLASH_SECTOR_SIZE;
	}
	return RES_OK;
}


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

static uint8_t tmp_buf[FLASH_ERASE_SIZE];  // 4KB临时缓冲

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
    // FATFS sector → 字节地址
    uint32_t byte_addr = sector * FLASH_SECTOR_SIZE   ;  // 512字节为单位

    // 字节地址 → Flash 4KB扇区号
    uint32_t flash_sector = byte_addr / FLASH_ERASE_SIZE;  // 4096字节为单位
    uint32_t erase_addr   = flash_sector * FLASH_ERASE_SIZE;
    uint32_t offset       = byte_addr - erase_addr;

    W25QXX_Read(erase_addr, tmp_buf, FLASH_ERASE_SIZE);
    memcpy(tmp_buf + offset, buff, count * FLASH_SECTOR_SIZE   );
    W25QXX_SectorErase(flash_sector);  // 传Flash扇区号
    W25QXX_Write_NoCheck(erase_addr, tmp_buf, FLASH_ERASE_SIZE);

    return RES_OK;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
	switch(cmd)
	{
		case CTRL_SYNC:
			break;
		case GET_SECTOR_SIZE:
			*(WORD*)buff = FLASH_SECTOR_SIZE;
			break;
		case GET_BLOCK_SIZE:
			*(WORD*)buff = FLASH_BLOCK_SIZE;
			break;
		case GET_SECTOR_COUNT:
			*(DWORD*)buff = FLASH_SECTOR_COUNT;
			break;
		default:
			break;
	}
	return RES_OK;
}