 
#include "gd32f470z_eval.h"

/* private variables */
static uint32_t GPIO_PORT[LEDn]           = {LED2_GPIO_PORT};
static uint32_t GPIO_PIN[LEDn]            = {LED2_PIN};
static rcu_periph_enum GPIO_CLK[LEDn]     = {LED2_GPIO_CLK};

static uint32_t KEY_PORT[KEYn]            = {USER_KEY_GPIO_PORT};
static uint32_t KEY_PIN[KEYn]             = {USER_KEY_PIN};
static rcu_periph_enum KEY_CLK[KEYn]      = {USER_KEY_GPIO_CLK};
static exti_line_enum KEY_EXTI_LINE[KEYn] = {USER_KEY_EXTI_LINE};
static uint8_t KEY_PORT_SOURCE[KEYn]      = {USER_KEY_EXTI_PORT_SOURCE};
static uint8_t KEY_PIN_SOURCE[KEYn]       = {USER_KEY_EXTI_PIN_SOURCE};
static uint8_t KEY_IRQn[KEYn]             = {USER_KEY_EXTI_IRQn};

/*!
    \brief      configure led GPIO
    \param[in]  led_num: specify the led to be configured
      \arg        LED2
    \param[out] none
    \retval     none
*/
void  gd_eval_led_init (led_typedef_enum led_num)
{
    /* enable the led clock */
    rcu_periph_clock_enable(GPIO_CLK[led_num]);

    /* configure led GPIO port */ 
    gpio_mode_set(GPIO_PORT[led_num], GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,GPIO_PIN[led_num]);
    gpio_output_options_set(GPIO_PORT[led_num], GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN[led_num]);

    GPIO_BC(GPIO_PORT[led_num]) = GPIO_PIN[led_num];
}

/*!
    \brief      turn on selected led
    \param[in]  led_num: specify the led to be turned on
      \arg        LED2
    \param[out] none
    \retval     none
*/
void gd_eval_led_on(led_typedef_enum led_num)
{
    GPIO_BOP(GPIO_PORT[led_num]) = GPIO_PIN[led_num];
}

/*!
    \brief      turn off selected led
    \param[in]  led_num: specify the led to be turned off
      \arg        LED2
    \param[out] none
    \retval     none
*/
void gd_eval_led_off(led_typedef_enum led_num)
{
    GPIO_BC(GPIO_PORT[led_num]) = GPIO_PIN[led_num];
}

/*!
    \brief      toggle selected led
    \param[in]  led_num: specify the led to be toggled
      \arg        LED2
    \param[out] none
    \retval     none
*/
void gd_eval_led_toggle(led_typedef_enum led_num)
{
    GPIO_TG(GPIO_PORT[led_num]) = GPIO_PIN[led_num];
}

/*!
    \brief      configure key
    \param[in]  key_num: specify the key to be configured
      \arg        KEY_USER: user key
    \param[in]  key_mode: specify button mode
      \arg        KEY_MODE_GPIO: key will be used as simple IO
      \arg        KEY_MODE_EXTI: key will be connected to EXTI line with interrupt
    \param[out] none
    \retval     none
*/
void gd_eval_key_init(key_typedef_enum key_num, keymode_typedef_enum key_mode)
{
    /* enable the key clock */
    rcu_periph_clock_enable(KEY_CLK[key_num]);
    rcu_periph_clock_enable(RCU_SYSCFG);

    /* configure button pin as input */
    gpio_mode_set(KEY_PORT[key_num], GPIO_MODE_INPUT, GPIO_PUPD_NONE,KEY_PIN[key_num]);

    if (key_mode == KEY_MODE_EXTI) {
        /* enable and set key EXTI interrupt to the lowest priority */
        nvic_irq_enable(KEY_IRQn[key_num], 2U, 0U);

        /* connect key EXTI line to key GPIO pin */
        syscfg_exti_line_config(KEY_PORT_SOURCE[key_num], KEY_PIN_SOURCE[key_num]);

        /* configure key EXTI line */
        exti_init(KEY_EXTI_LINE[key_num], EXTI_INTERRUPT, EXTI_TRIG_FALLING);
        exti_interrupt_flag_clear(KEY_EXTI_LINE[key_num]);
    }
}

/*!
    \brief      return the selected key state
    \param[in]  key: specify the key to be checked
      \arg        KEY_USER: user key
    \param[out] none
    \retval     the key's GPIO pin value
*/
uint8_t gd_eval_key_state_get(key_typedef_enum key)
{
    return gpio_input_bit_get(KEY_PORT[key], KEY_PIN[key]);
}
