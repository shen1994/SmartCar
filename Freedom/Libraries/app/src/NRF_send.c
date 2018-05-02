/**
*     COPYRIGHT NOTICE
*     All rights reserved.
*
* @file         NRF_send.c
* @brief        NRF中断式传递
*
* NRF中断式传递
*
* @version 1.0
* @author  hejun
* @date    2014年11月27日
*
*
*     修订说明：最初版本
*/
#include "NRF_send.h"
#include "OLED.h"
extern int kinetis_spi_bus_init(struct spi_bus *bus, uint32_t instance);
static NRF_CallBackType NRF_CallBackTable[1] = {NULL};

/**
*	@brief NRF回调函数注册
*	@parma[in] 回调函数
*	@return  无
*/
Status NRF_CallbackInstall(NRF_CallBackType AppCBFun)
{
    if(AppCBFun != NULL)
    {
        NRF_CallBackTable[0] = AppCBFun;
        return OK;
    }
    return FALSE;
}

/**NRF接收缓冲区*/
uint8_t NRF2401RXBuffer[NRF_data_length];
/**
*	@brief NRF中断服务函数
*	@parma[in] 引脚号
*	@return  无
*/
void NRF_ISR(uint32_t array)
{
    uint32_t len = 0;
    uint8_t status = 0;
    if(array & (1 << NRF_irq_pin))
    {
        status = read_reg(STATUS);
        if(status & STATUS_MAX_RT_MASK || (status & 0x01))
        {
            nrf24l01_FlushTX();
            write_reg(STATUS, status | STATUS_MAX_RT_MASK);
        }
        if(status & STATUS_TX_DS_MASK)
        {
            write_reg(STATUS, status | STATUS_TX_DS_MASK | STATUS_MAX_RT_MASK);
        }
        if(status & STATUS_RX_DR_MASK) //读准备
        {
            write_reg(STATUS, status | STATUS_RX_DR_MASK);
            nrf24l01_read_packet(NRF2401RXBuffer, &len);
            if(NRF_CallBackTable[0])
            {
                NRF_CallBackTable[0](NRF2401RXBuffer, len);
            }
        }

        if((status & STATUS_MAX_RT_MASK || (status & 0x01)) || (status & STATUS_TX_DS_MASK))
        {
            nrf24l01_set_rx_mode();
        }
    }
}


/**
*	@brief NRF引脚,中断初始化
*
*/
Status NRF_Gpio_Init(void)
{
    PORT_PinMuxConfig(HW_GPIOA, 16, kPinAlt2);
    PORT_PinMuxConfig(HW_GPIOA, 15, kPinAlt2);
    PORT_PinMuxConfig(HW_GPIOA, 14, kPinAlt2);
    PORT_PinMuxConfig(HW_GPIOA, 17, kPinAlt2);
    /* 初始化2401所需的CE引脚 */
    GPIO_QuickInit(HW_GPIOA, 24 , kGPIO_Mode_OPP);
    /* 初始化2401模块*/
    static struct spi_bus bus;
    kinetis_spi_bus_init(&bus, HW_SPI0);
    nrf24l01_init(&bus, 0);
    //检测是否存在无线设备，并配置接收和发送地址
    if(nrf24l01_probe())
    {
        printf("no nrf24l01 device found!\r\n");
    }
    GPIO_QuickInit(NRF_irq_port, NRF_irq_pin, kGPIO_Mode_IPU); /*设置IRQ上拉*/
    /* 设置NRF外部引脚中断回调函数 */
    GPIO_CallbackInstall(NRF_irq_port, NRF_ISR);
    /* 引脚的中断 上升沿触发 */
    GPIO_ITDMAConfig(NRF_irq_port, NRF_irq_pin, kGPIO_IT_FallingEdge );
    return OK;
}

/**
*	@brief NRF接收中断函数
*	@return  无
*/
Status Data_Rec_Analyse(uint8_t *rxbuff, uint32_t len)
{
    if(len == 14) //长度满足协议
    {
        AnalysisData(rxbuff);
    }
    return OK;
}

/**
*	@brief NRF发送图像
*	@return  无
*/
Status NRF_Send_Image(uint8_t image[][180], int HIGH, int WIDTH)
{
    int Send_Count = 0;
    int Last_lenth = 0;
    uint8_t Data_Header[2] = {0x01, 0xfe};
    uint8_t Data_Tail[2]  = {0xfe, 0x01};

    nrf24l01_write_packet(Data_Header, 2);
    Send_Count = HIGH * WIDTH / 32;
    Last_lenth = (HIGH * WIDTH) % 32;
    for(int count = 0; count < Send_Count; count++)
        nrf24l01_write_packet(*image + 32 * count, 32);
    if(Last_lenth != 0)
        nrf24l01_write_packet(*image + 32 * Send_Count, Last_lenth);
    nrf24l01_write_packet(Data_Tail, 2);
    return OK;
}
