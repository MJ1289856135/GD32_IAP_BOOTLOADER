#ifndef __BOOTLOAD_DRV_H
#define __BOOTLOAD_DRV_H

#include "gd32f4xx.h"

typedef struct
{
    uint32_t flag;          // 标志位
    uint32_t file_size;     // 文件大小
    char filename[64];      // 文件名
} UpdateInfo_t;

void Bootloader_Main(void);
void RunApp(void);
void Print_File_Content(void);
//void FSInit(void);

#endif
