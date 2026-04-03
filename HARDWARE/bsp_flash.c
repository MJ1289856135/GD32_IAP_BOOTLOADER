#include "bsp_flash.h"
#include <gd32f4xx_fmc.h>

/**
 * @description: 初始化MCU内部Flash操作模块
 * @return {*}
 */
void Flash_IF_Init(void)
{
    fmc_unlock();

    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR | FMC_FLAG_RDDERR | FMC_FLAG_BUSY);
}

void Flash_IF_Finish(void)
{
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR | FMC_FLAG_RDDERR | FMC_FLAG_BUSY);
	fmc_lock();
}

int Flash_IF_App_Erase(int StartSectorId, int EndSectorId)
{
    int sectorIdIndex = StartSectorId;

    while (sectorIdIndex <= EndSectorId)
    {
        if (FMC_READY == fmc_sector_erase(CTL_SN(sectorIdIndex)))
        {
            sectorIdIndex++;
        }
        else
        {
            return -1;
        }
    }

    return 0;
}

/**
 * @brief 按照4字节单位将缓存区数据烧写到Flash
 *
 * @param address [in out] 传输地址的指针
 * @param buffer [in] 数据缓存区
 * @param length [in] 数据缓存区长度
 * @return int 0 正确 -1 烧写错误 -2 回读错误
 */
int Flash_IF_Write(__IO uint32_t *address, uint32_t *buffer, int length)
{
    while (length > 0)
    {
        if (FMC_READY == fmc_word_program(*address, *buffer))
        {
            if (*buffer != REG32(*address))
            {

                return -2;
            }
            *address += 4;
            buffer++;
            length--;
        }
        else
        {
            return -1;
        }
    }
    return 0;
}