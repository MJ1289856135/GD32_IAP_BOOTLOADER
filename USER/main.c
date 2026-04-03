#include "gd32f4xx.h"
#include "bsp_led.h"
#include "systick.h"
#include "w25qxx_drv.h"
#include "bsp_usart.h"
#include "bootLoader_drv.h"
#include "ff.h"


int main(void)
{
	systick_config();
    AllUsartInit();
	RunApp();
    while (1)
    {
        LED0 = 1;
        delay_1ms(1000);
        LED0 = 0;
        delay_1ms(1000);
    }
}
