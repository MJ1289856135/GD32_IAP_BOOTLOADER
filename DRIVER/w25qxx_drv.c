#include "w25qxx_drv.h"
#include "bsp_usart.h"

/**
 * @brief  发送写使能命令
 * @note   所有写操作前必须调用
 */
static void W25QXX_WriteEnable(void)
{
    SPI2_CS = 0;
    SpiReadWriteByte(W25Q_CMD_WRITE_ENABLE);
    SPI2_CS = 1;
}

/**
 * @brief  查询 W25QXX 是否忙
 * @retval 0 = 空闲, 1 = 忙
 */
uint8_t W25QXX_IsBusy(void)
{
    uint8_t status;

    SPI2_CS = 0;
    SpiReadWriteByte(W25Q_CMD_READ_STATUS1);
    status = SpiReadWriteByte(0xFF);
    SPI2_CS = 1;

    return (status & 0x01);
}

/**
 * @brief  等待 Flash 空闲，带超时保护
 */
static void W25QXX_WaitBusy(void)
{
    uint32_t timeout = 200000;
    while(W25QXX_IsBusy())
    {
        if(--timeout == 0)
        {
            LogPrintf("WaitBusy TIMEOUT!\r\n");
            return;
        }
    }
}

/**
 * @brief  读取 W25QXX JEDEC ID
 * @retval Flash ID (24-bit)
 */
uint32_t W25QXX_ReadID(void)
{
    uint32_t Id = 0;

    SPI2_CS = 0;
    SpiReadWriteByte(W25Q_CMD_JEDEC_ID);

    Id |= SpiReadWriteByte(0xFF) << 16;
    Id |= SpiReadWriteByte(0xFF) << 8;
    Id |= SpiReadWriteByte(0xFF);

    SPI2_CS = 1;

    return Id;
}

/**
 * @brief  从指定地址读取数据
 * @param  Address  起始地址
 * @param  Buffer   数据缓冲区
 * @param  Length   读取长度
 */
void W25QXX_Read(uint32_t Address, uint8_t *Buffer, uint32_t Length)
{
    SPI2_CS = 0;

    SpiReadWriteByte(W25Q_CMD_READ_DATA);

    SpiReadWriteByte((Address >> 16) & 0xFF);
    SpiReadWriteByte((Address >> 8) & 0xFF);
    SpiReadWriteByte(Address & 0xFF);

    for (uint32_t i = 0; i < Length; i++)
    {
        Buffer[i] = SpiReadWriteByte(0xFF);
    }

    SPI2_CS = 1;

}

/**
 * @brief  分页写入（单页 256 字节）
 * @param  Address  起始地址
 * @param  Buffer   数据缓冲区
 * @param  Length   读取长度
 */
void W25QXX_Write_Page(uint32_t Address, const uint8_t *Buffer, uint16_t Length)
{
    if (Length > 256) Length = 256;

    W25QXX_WriteEnable();

    SPI2_CS = 0;

    SpiReadWriteByte(W25Q_CMD_PAGE_PROGRAM);

    SpiReadWriteByte((Address >> 16) & 0xFF);
    SpiReadWriteByte((Address >> 8) & 0xFF);
    SpiReadWriteByte(Address & 0xFF);

    for (uint16_t i = 0; i < Length; i++)
    {
        SpiReadWriteByte(Buffer[i]);
    }

    SPI2_CS = 1;

    W25QXX_WaitBusy();
}

/**
 * @brief  无检查连续写入（可跨页）
 * @param  Address  起始地址
 * @param  Buffer   数据缓冲区
 * @param  Length   读取长度
 */
void W25QXX_Write_NoCheck(u32 WriteAddr,u8* pBuffer,u16 NumByteToWrite)
{
	uint16_t pagereMain =256 - WriteAddr % 256; //单页剩余的字节数;

	if(NumByteToWrite <= pagereMain)
	{
		pagereMain = NumByteToWrite;//不大于256个字节
	}

	while(1)
	{
		W25QXX_Write_Page(WriteAddr,pBuffer,pagereMain);
		if(NumByteToWrite == pagereMain)
		{
			break;//写入结束了
		}
		else
		{
			pBuffer+=pagereMain;
			WriteAddr+=pagereMain;

			NumByteToWrite-=pagereMain;			  //减去已经写入了的字节数
			if(NumByteToWrite>256)
			{
				pagereMain=256; //一次可以写入256个字节
			}
			else
			{
				pagereMain=NumByteToWrite; 	  //不够256个字节了
			}
		}
	}
}

uint8_t W25QXX_BUFFER[4096];
/**
 * @brief  扇区写入（自动擦除 & 跨扇区处理）
 * @param  Address  起始地址
 * @param  Buffer   数据缓冲区
 * @param  Length   读取长度
 */
void W25QXX_Write(uint32_t WriteAddr,uint8_t* pBuffer,uint16_t NumByteToWrite)
{
	uint32_t sectorPos = WriteAddr / 4096;//扇区地址;
	uint16_t sectorOff = WriteAddr % 4096;//在扇区内的偏移;
	uint16_t sectorMain = 4096 - sectorOff;//扇区剩余空间大小;
 	uint16_t i;
   	uint8_t * W25QXX_BUF = W25QXX_BUFFER;

 	if(NumByteToWrite<=sectorMain)
 	{
 		sectorMain = NumByteToWrite;//不大于4096个字节
 	}

	while(1)
	{
		W25QXX_Read(sectorPos * 4096, W25QXX_BUF, 4096);

		for(i=0;i<sectorMain;i++)//校验数据
		{
			if(W25QXX_BUF[sectorOff+i]!=0XFF)break;//需要擦除
		}
		if(i<sectorMain)//需要擦除
		{
			W25QXX_SectorErase(sectorPos);//擦除这个扇区
			for(i=0;i<sectorMain;i++)	   //复制
			{
				W25QXX_BUF[i+sectorOff]=pBuffer[i];
			}
			W25QXX_Write_NoCheck(sectorPos*4096,W25QXX_BUF,4096);//写入整个扇区

		}
		else
		{
			W25QXX_Write_NoCheck(WriteAddr,pBuffer,sectorMain);//写已经擦除了的,直接写入扇区剩余区间.
		}

		if(NumByteToWrite == sectorMain)
		{
			break;//写入结束了
		}
		else//写入未结束
		{
			sectorPos++;//扇区地址增1
			sectorOff=0;//偏移位置为0

		   	pBuffer+=sectorMain;  //指针偏移
			WriteAddr+=sectorMain;//写地址偏移
		   	NumByteToWrite-=sectorMain;				//字节数递减
			if(NumByteToWrite>4096)
			{
				sectorMain=4096;	//下一个扇区还是写不完
			}
			else
			{
				sectorMain=NumByteToWrite;			//下一个扇区可以写完了
			}
		}
	}
}

/**
 * 擦除扇区（4KB）
 * @param Address
 */
void W25QXX_SectorErase(uint32_t Address)
{

	Address *= 4096;
	W25QXX_WriteEnable();
	W25QXX_WaitBusy();
    SPI2_CS = 0;

    SpiReadWriteByte(W25Q_CMD_SECTOR_ERASE);

    SpiReadWriteByte((Address >> 16) & 0xFF);
    SpiReadWriteByte((Address >> 8) & 0xFF);
    SpiReadWriteByte(Address & 0xFF);

    SPI2_CS = 1;

    W25QXX_WaitBusy();

}

/**
 * 批量写入（分页 + 扇区）
 * @param Address
 * @param Buffer
 * @param Length
 */
void W25QXX_WriteBuffer(uint32_t Address, uint8_t *Buffer, uint32_t Length)
{
    uint32_t pageRemain;
    uint32_t writeSize;

    while (Length > 0)
    {
        pageRemain = 256 - (Address % 256);

        writeSize = (Length < pageRemain) ? Length : pageRemain;

        W25QXX_Write(Address, Buffer, writeSize);

        Address += writeSize;
        Buffer  += writeSize;
        Length  -= writeSize;
    }
}

/**
 *准备就绪检查
 * @return
 */
uint8_t W25QXX_IsReady(void)
{
    return 1; // 简化版：直接认为ready
}

/**
 * Flash 初始化
 */
void W25QXXInit(void)
{
    SpiInit();

    uint32_t Id = W25QXX_ReadID();

    if (Id != 0xEF4018)
    {
        LogPrintf("error W25QXX ID = 0x%x\r\n", Id);
        return;
    }
    LogPrintf("success W25QXX ID = 0x%x\r\n", Id);
}

uint8_t TxBuf[256];
uint8_t RxBuf[256];

/**
 * Flash 测试
 */
void FlashTest()
{
    for (int i = 0; i < 256; i++)
    {
        TxBuf[i] = i;
    }

    /* 擦除 */
    W25QXX_SectorErase(0x000000);

    /* 写入 */
    W25QXX_Write(0x000000, TxBuf, 256);

    /* 读取 */
    W25QXX_Read(0x000000, RxBuf, 256);

    /* 校验 */
    if (memcmp(TxBuf, RxBuf, 256) == 0)
    {
        printf("Flash OK\r\n");
    }
    else
    {
        printf("Flash FAIL\r\n");
    }
}


