#include "bsp_led.h"


/***************************************************************
  *  @brief     指示灯初始化
  *  @param     参数   
  *  @return 
  *  @note      备注
  *  @Sample usage:     函数的使用方法 
 **************************************************************/
void InitLED(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_14);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_14);
    LED0 = 0;
}

void ToggleLED0(void)
{
    gpio_bit_toggle(GPIOB,GPIO_PIN_14);
}

