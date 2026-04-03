#include "bootLoader_drv.h"
#include "bsp_usart.h"
#include "systick.h"
#include "w25qxx_drv.h"
#include "ff.h"
#include "bsp_flash.h"

#define APP1_ADDR    0x08040000
#define APP2_ADDR    0x08080000
#define APP_MAX_SIZE 0x00060000 // 384KB

#define SRAM_START 0x20000000
#define SRAM_END   0x20040000  // GD32F470 128KB*2 banks = 256KB 或 384KB, 根据实际

#define UPDATE_FLAG_ADDR   0x08030000   // 示例：Bootloader保留区
#define UPDATE_FLAG_VALUE  0xA5A5A5A5



FATFS fs;
uint8_t work[4096];
/*-----------------------------------------------
 * 文件系统初始化
 * 挂载 FatFs，如果没有文件系统则格式化
 *-----------------------------------------------*/
void FSInit(void)
{
    FRESULT res;

    LogPrintf("[INFO] === FS Init Start ===\r\n");
    // 尝试挂载
    res = f_mount(&fs, "0:", 1);
    if(res == FR_OK)
    {
        LogPrintf("[INFO] FatFs mount OK\r\n");
    }
    else if(res == FR_NO_FILESYSTEM)
    {
        LogPrintf("[INFO] No FS found, formatting...\r\n");

        // 格式化 0: 盘，opt 为 NULL，使用 512 字节缓冲区
        MKFS_PARM opt;
		memset(&opt, 0, sizeof(opt));
		opt.fmt = FM_FAT | FM_SFD;  // FM_SFD = 无MBR，引导扇区直接在sector0
		opt.n_fat = 1;
		opt.align = 1;
		opt.n_root = 512;
		opt.au_size = 0;

		res = f_mkfs("0:", &opt, work, sizeof(work));
        if(res == FR_OK)
        {
            LogPrintf("[INFO] mkfs OK\r\n");
            // 格式化后重新挂载
            res = f_mount(&fs, "0:", 1);
            LogPrintf("[INFO] Remount: %d\r\n", res);
        }
        else
        {
            LogPrintf("[ERROR] mkfs error: %d\r\n", res);
        }
    }
    else
    {
        LogPrintf("[ERROR] Mount error: %d\r\n", res);
    }

    LogPrintf("[INFO] === FS Init End ===\r\n");
}

/*-----------------------------------------------
 * Bootloader 初始化
 * 初始化 Flash、文件系统和相关硬件
 *-----------------------------------------------*/
void Bootloader_Init(void)
{
    LogPrintf("[INFO] === Bootloader Start ===\r\n");
    // 初始化内部 Flash 操作
    Flash_IF_Init();\
	FSInit();
    LogPrintf("[INFO] === Bootloader Init Done ===\r\n");
}

/*-----------------------------------------------
 * 根据 Flash 地址获取扇区号
 * GD32F4 内部 Flash 分扇区管理
 *-----------------------------------------------*/
int GetSectorId(uint32_t addr)
{
    if(addr < 0x08004000) return 0;
    else if(addr < 0x08008000) return 1;
    else if(addr < 0x0800C000) return 2;
    else if(addr < 0x08010000) return 3;
    else if(addr < 0x08020000) return 4;
    else if(addr < 0x08040000) return 5;
    else if(addr < 0x08060000) return 6;
    else if(addr < 0x08080000) return 7;
    else if(addr < 0x080A0000) return 8;
    else if(addr < 0x080C0000) return 9;
    else if(addr < 0x080E0000) return 10;
    else return 11;
}

/*-----------------------------------------------
 * 将 bin 文件烧写到 APP 区
 * 参数：
 *   app_addr : APP 起始 Flash 地址
 *   filename : bin 文件名
 * 返回值：
 *   0 成功
 *  -1 失败
 *-----------------------------------------------*/
int Bootloader_FlashApp(uint32_t app_addr,const char *filename)
{
    FIL file;
    if(f_open(&file, filename, FA_READ) != FR_OK){
        LogPrintf("[ERROR] Open file '%s' failed!\r\n", filename);
        return -1;
    }

    UINT br;
    uint8_t buf[256];
    uint32_t flash_addr = app_addr;

    // 擦除 APP 区
    int start_sector = GetSectorId(app_addr);           // 需要实现 APP 地址 -> 扇区映射
    int end_sector   = GetSectorId(app_addr + APP_MAX_SIZE - 1);
    if(Flash_IF_App_Erase(start_sector, end_sector) != 0){
        LogPrintf("[ERROR] Flash erase failed!\r\n");
        f_close(&file);
        return -1;
    }

    // 写入 Flash
    while(f_read(&file, buf, sizeof(buf), &br) == FR_OK && br > 0){
        int word_len = br / 4;
        uint32_t *pbuf = (uint32_t*)buf;

        if(Flash_IF_Write(&flash_addr, pbuf, word_len) != 0){
            LogPrintf("[ERROR] Flash write failed!\r\n");
            f_close(&file);
            return -1;
        }

        // 处理剩余字节
        int remain = br % 4;
        if(remain){
            uint32_t last_word = 0xFFFFFFFF;
            memcpy(&last_word, buf + word_len*4, remain);
            if(Flash_IF_Write(&flash_addr, &last_word, 1) != 0){
                LogPrintf("[ERROR] Flash write last bytes failed!\r\n");
                f_close(&file);
                return -1;
            }
        }
    }

    f_close(&file);
    LogPrintf("[INFO] Flash write done to 0x%08lX\r\n", app_addr);
    return 0;
}

/*-----------------------------------------------
 * 系统反初始化
 * 关闭外设、中断，复位 RCC
 *-----------------------------------------------*/
void System_DeInit(void)
{
    // 复位 RCC
    rcu_deinit();

    // 关闭中断() app程序记得使能
    __disable_irq();
}


/*-----------------------------------------------
 * 跳转到 APP
 * 参数：
 *   app_addr : APP 起始地址
 *-----------------------------------------------*/
void JumpToApp(uint32_t app_addr)
{
    uint32_t sp = *(volatile uint32_t*)app_addr;
    uint32_t reset = *(volatile uint32_t*)(app_addr + 4);

	LogPrintf("[INFO] SP    = 0x%08X\r\n", sp);
	LogPrintf("[INFO] RESET = 0x%08X\r\n", reset);
	if ((sp < SRAM_START) || (sp > SRAM_END)) {
		LogPrintf("[ERROR] Invalid APP SP!\r\n");
		return;
	}

	if ((reset < APP1_ADDR) || (reset > (APP1_ADDR + APP_MAX_SIZE))) {
		LogPrintf("[ERROR] Invalid APP RESET!\r\n");
		return;
	}
	
	System_DeInit();
	usart_disable(EVAL_COM0);
	gpio_deinit(USART_0_PORT);
	Flash_IF_Finish();
    rcu_deinit();   
    __set_MSP(sp);
	SCB->VTOR = app_addr;
	
    typedef void (*pfunc)(void);
    pfunc app_reset = (pfunc)(reset);
    app_reset();
}



/*-----------------------------------------------
 * 检查 APP 是否存在（有效性）
 * 参数：
 *   app_addr : APP 起始地址
 * 返回值：
 *   1 : 存在且有效
 *   0 : 无效
 *-----------------------------------------------*/
int ExistApplication(uint32_t app_addr)
{
    uint32_t sp    = *(uint32_t*)app_addr;
    uint32_t reset = *(uint32_t*)(app_addr + 4);

    if ((sp < SRAM_START) || (sp > SRAM_END))
        return 0;

    if ((reset < APP1_ADDR) || (reset > (APP1_ADDR + APP_MAX_SIZE)))
        return 0;

    return 1;
}

/*-----------------------------------------------
 * 启动 APP 或升级
 * 检查更新标志，如果需要则烧写 APP，否则直接跳转
 *-----------------------------------------------*/
void RunApp(void)
{
    UpdateInfo_t *check = (UpdateInfo_t*)UPDATE_FLAG_ADDR;
	LogPrintf("[INFO] -------------------------------------\r\n");
	LogPrintf("[INFO] ----------This is BootLoader---------\r\n");
	LogPrintf("[INFO] -------------------------------------\r\n");

    // 1有升级请求
    if(check->flag == UPDATE_FLAG_VALUE)
    {
        LogPrintf("[INFO] Start updating APP...\r\n");

        Flash_IF_Init();

        if(Bootloader_FlashApp(APP1_ADDR,check->filename) == 0)
        {
            LogPrintf("[INFO] Update success!\r\n");

            // 清标志（必须成功才清）
            if (Flash_IF_App_Erase(GetSectorId(UPDATE_FLAG_ADDR), GetSectorId(UPDATE_FLAG_ADDR)) != 0)
            {
                LogPrintf("[ERROR] Erase flag sector failed!\r\n");
            }

            Flash_IF_Finish();

            // 写完后再检查 APP
            if(ExistApplication(APP1_ADDR))
            {
                JumpToApp(APP1_ADDR);
            }
            else
            {
                LogPrintf("[ERROR] APP invalid after update!\r\n");
                while(1);
            }
        }
    }

    // 正常启动
    if(ExistApplication(APP1_ADDR))
    {
        JumpToApp(APP1_ADDR);
    }
    else
    {
        LogPrintf("No valid APP! Stay in Bootloader.\r\n");

        while(1);  // 等待下载
    }
}


