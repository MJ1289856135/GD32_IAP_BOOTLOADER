#include "bsp_spi.h"
#include "bsp_usart.h"
uint8_t SpiReadWriteByte(uint8_t data)
{
    while (!spi_i2s_flag_get(SPI2, SPI_FLAG_TBE));
    spi_i2s_data_transmit(SPI2, data);

    while (!spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE));
    return spi_i2s_data_receive(SPI2);
}

void SpiInit(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_SPI2);

    /* GPIO∏¥”√ */
    gpio_af_set(GPIOC, GPIO_AF_6, GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);

    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP,
                  GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);

    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                          GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);

    /* NSS PD0 */
    gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_0);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);

    SPI2_CS = 1;

    spi_parameter_struct t_spi_parameter_struct;

    spi_i2s_deinit(SPI2);

    t_spi_parameter_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
    t_spi_parameter_struct.device_mode  = SPI_MASTER;
    t_spi_parameter_struct.endian       = SPI_ENDIAN_MSB;
    t_spi_parameter_struct.frame_size   = SPI_FRAMESIZE_8BIT;
    t_spi_parameter_struct.nss          = SPI_NSS_SOFT;
    t_spi_parameter_struct.prescale     = SPI_PSC_32;
    t_spi_parameter_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;

    spi_init(SPI2, &t_spi_parameter_struct);
    spi_enable(SPI2);
    SpiReadWriteByte(0xFF);
}



