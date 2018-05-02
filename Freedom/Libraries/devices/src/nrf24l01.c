/**
  ******************************************************************************
  * @file    nrf24l01.c
  * @author  YANDLD
  * @version V2.5
  * @date    2013.12.25
  * @brief   www.beyondcore.net   http://upcmcu.taobao.com
  * @note    此文件为NRF24L01无线模块的驱动，支持台产和挪威产芯片
  ******************************************************************************
  */

#include "nrf24l01.h"
#include "spi_abstraction.h"
#include "spi.h"
#include "uart.h"

#define NRF24L01_DEBUG		0
#if ( NRF24L01_DEBUG == 1 )
#define NRF24L01_TRACE	printf
#else
#define NRF24L01_TRACE(...)
#endif

/* STATUS Bit Fields */
#define STATUS_RX_P_NO_SHIFT        1
#define STATUS_RX_P_NO_MASK         0x0Eu
#define STATUS_RX_P_NO(x)           (((uint8_t)(((uint32_t)(x))<<STATUS_RX_P_NO_SHIFT))&STATUS_RX_P_NO_MASK)
#define STATUS_TX_FULL              0x01u
/* CONFIG bit Fields */
#define CONFIG_PRIM_RX_MASK         0x01u
#define CONFIG_PWR_UP_MASK          0x02u
#define CONFIG_CRCO_MASK            0x04u
#define CONFIG_EN_CRC_MASK          0x08u
#define CONFIG_MASK_MAX_RT_MASK     0x10u
#define CONFIG_MASK_TX_DS_MASK      0x20u
#define CONFIG_MASK_RX_DS_MASK      0x40u
/* OBSERVE_TX bit Fields */
#define OBSERVE_TX_ARC_CNT_MASK     0x0Fu
#define OBSERVE_TX_ARC_CNT_SHIFT    0
#define OBSERVE_TX_PLOS_CNT_MASK    0xF0u
#define OBSERVE_TX_PLOS_CNT_SHIFT   4
//********************************************************************************************************************//
/* SPI(nRF24L01) 指令 */
#define READ_REG                    0x00   // 读配置寄存器
#define WRITE_REG                   0x20   // 写配置寄存器
#define RD_RX_PLOAD                 0x61   // 读取RX FIFO中的数据
#define WR_TX_PLOAD                 0xA0   // 向TX FIFO中写入数据
#define FLUSH_TX                    0xE1   // 清除TX FIFO中的数据 应用于发射模式下
#define FLUSH_RX                    0xE2   // 清除RX FIFO中的数据 应用于接收模式下
#define REUSE_TX_PL                 0xE3   // 重新使用上一包有效数据
#define R_RX_PL_WID                 0x60
#define NOP                         0xFF   // 保留
//********************************************************************************************************************//
/* register define */
#define CONFIG              0x00  //配置发送状态，CRC校验模式以及发收发状态响应方式
#define EN_AA               0x01  //自动应答功能设置
#define EN_RXADDR           0x02  //可用信道设置
#define SETUP_AW            0x03  //收发地址宽度设置
#define SETUP_RETR          0x04  //自动重发功能设设置
#define RF_CH               0x05  //工作频率设定
#define RF_SETUP            0x06

#define OBSERVE_TX          0x08
#define CD                  0x09
#define RX_ADDR_P0          0x0A
#define RX_ADDR_P1          0x0B
#define RX_ADDR_P2          0x0C
#define RX_ADDR_P3          0x0D
#define RX_ADDR_P4          0x0E
#define RX_ADDR_P5          0x0F
#define TX_ADDR             0x10
#define RX_PW_P0            0x11
#define RX_PW_P1            0x12
#define RX_PW_P2            0x13
#define RX_PW_P3            0x14
#define RX_PW_P4            0x15
#define RX_PW_P5            0x16
#define FIFO_STATUS         0x17
#define PYNPD               0x1C
#define FEATURE             0x1D

static struct spi_device device;
const uint8_t TX_ADDRESS[5] = {0x34, 0x43, 0x10, 0x10, 0x01}; //发送地址
const uint8_t RX_ADDRESS[5] = {0x34, 0x43, 0x10, 0x10, 0x01}; //接收地址

//读寄存器
uint8_t read_reg(uint8_t addr)
{
    uint8_t temp = READ_REG + addr;
    spi_write(&device, &temp, 1, false);
    spi_read(&device, &temp, 1, true);
    return temp;
}
//写寄存器
void write_reg(uint8_t addr, uint8_t val)
{
    uint8_t temp = WRITE_REG + addr;
    spi_write(&device, &temp, 1, false);
    spi_write(&device, &val, 1, true);
}
//写寄存器
void NRF_Write_Reg(uint8_t addr, uint8_t val)
{
    uint8_t temp = addr;
    spi_write(&device, &temp, 1, false);
    spi_write(&device, &val, 1, true);
}
//写数据
static void write_buffer(uint8_t addr, uint8_t *buf, uint32_t len)
{
    uint8_t temp = WRITE_REG + addr;
    spi_write(&device, &temp, 1, false);
    spi_write(&device, buf, len, true);
}
//读数据
static void read_buffer(uint8_t addr, uint8_t *buf, uint32_t len)
{
    uint8_t temp = READ_REG + addr;
    spi_write(&device, &temp, 1, false);
    spi_read(&device, buf, len, true);
}
//NRF设备检测，并配置接收和发送地址
int nrf24l01_probe(void)
{
    uint8_t i;
    uint8_t buf[5];
    read_buffer(TX_ADDR, buf, 5);
    for(i = 0; i < 5; i++)
    {
        if((buf[i] != 0) && (buf[i] != 0xFF))
        {
            NRF24L01_CE_LOW();
            /* init sequence */
            write_reg(CONFIG, 0x0F); /* config */   //?????
            write_reg(EN_AA, 0x01);/* aa */         //为什么开2个管道
            write_reg(EN_RXADDR, 0x01); /* available pipe */
            write_reg(SETUP_AW, 0x03);  /* setup_aw */
            write_reg(SETUP_RETR, 0x05);/* setup_retr */  //500us  7count
            write_reg(RF_CH, 40); /* RF freq */
            write_reg(RF_SETUP, 0X06);//0dbm 1mbps
            write_reg(PYNPD, 0x01); /* enable dynmic playload whhich means packet len is varible */
            write_reg(FEATURE, 0x06); /* enable dynmic playload whhich means packet len is varible */
            write_buffer(RX_ADDR_P0, (uint8_t *)RX_ADDRESS, 5);
            write_buffer(TX_ADDR, (uint8_t *)TX_ADDRESS, 5);
            return 0;
        }
    }
    return 1;
}
//NRF模块初始化
int nrf24l01_init(spi_bus_t bus, uint32_t cs)
{
    uint32_t ret;
    device.csn = cs;
    device.config.baudrate = 1 * 1000 * 1000;
    device.config.data_width = 8;
    device.config.mode = SPI_MODE_0 | SPI_MASTER | SPI_MSB;
    ret = spi_bus_attach_device(bus, &device);
    if(ret)
    {
        return ret;
    }
    else
    {
        ret = spi_config(&device);
    }
    return ret;
}


//该函数初始化NRF24L01到TX模式
//设置TX地址,写TX数据宽度,设置RX自动应答的地址,填充TX发送数据,选择RF频道,波特率和LNA HCURR
//PWR_UP,CRC使能
//当CE变高后,即进入RX模式,并可以接收数据了
//CE为高大于10us,则启动发送.

void nrf24l01_set_tx_mode(void)
{
    uint8_t value;
    NRF24L01_CE_LOW();
    write_reg(CONFIG, 0x8E); /* Set PWR_UP bit, enable CRC(2 length) & Prim:RX. RX_DR enabled.. */
    NRF_Write_Reg(FLUSH_TX, 0XFF);
    NRF_Write_Reg(FLUSH_RX, 0XFF);
    value = 0x50;
    spi_write(&device, &value, 1, true);
    value = 0x73;
    spi_write(&device, &value, 1, true);
    write_reg(PYNPD, 0x01);
    write_reg(FEATURE, 0x06);
    NRF24L01_CE_HIGH();
}

//该函数初始化NRF24L01到RX模式
//设置RX地址,写RX数据宽度,选择RF频道,波特率和LNA HCURR
//当CE变高后,即进入RX模式,并可以接收数据了
void nrf24l01_set_rx_mode(void)
{
    uint8_t value;
    NRF24L01_CE_LOW();
    write_reg(CONFIG, 0x0f);
    NRF_Write_Reg(FLUSH_TX, 0XFF);
    NRF_Write_Reg(FLUSH_RX, 0XFF);
    value = 0x50;
    spi_write(&device, &value, 1, true);
    value = 0x73;
    spi_write(&device, &value, 1, true);
    write_reg(PYNPD, 0x01);
    write_reg(FEATURE, 0x06);
    NRF24L01_CE_HIGH();
}

//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:发送完成状况
int nrf24l01_write_packet(uint8_t *buf, uint32_t len)
{
    NRF24L01_CE_LOW();
    write_reg(RF_CH, 40);
    uint8_t value = 0xa8;
    spi_write(&device, &value, 1, false);
    spi_write(&device, buf, len, true);
    NRF24L01_CE_HIGH();
		return 0;
}

//NRF读一帧数据
int nrf24l01_read_packet(uint8_t *buf, uint32_t *len)
{
    uint8_t sta, rev_len;
    /* read len and data */
    rev_len = read_reg(R_RX_PL_WID);
    read_buffer(RD_RX_PLOAD, buf, rev_len);
    *len = rev_len;
    /* if rev_len > 32 which means a error occur, usr FLUSH_RX to clear */
    if(rev_len > 32)
    {
        NRF24L01_TRACE("rev len > 32, error!\r\n");
        sta = FLUSH_RX;
        spi_write(&device, &sta, 1, true);
        *len = 32;
        return 2;
    }
    return 0;
}
//清空接受缓冲区
void nrf24l01_FlushRX(void)
{
    NRF24L01_CE_LOW();
    NRF_Write_Reg(FLUSH_RX, 0XFF);
    NRF24L01_CE_HIGH();
    DelayUs(130);
}

//清空发送缓冲区
void nrf24l01_FlushTX(void)
{
    NRF24L01_CE_LOW();
    NRF_Write_Reg(FLUSH_TX, 0XFF);
    NRF24L01_CE_HIGH();
}

//读取Status
char CheckStatus(void)
{
    uint8_t status;
    status = read_reg(STATUS);
    /* clear pendign bit */
    if(status & STATUS_RX_DR_MASK) //读准备
        write_reg(STATUS, status | STATUS_RX_DR_MASK);
    if(status & STATUS_MAX_RT_MASK)
        write_reg(STATUS, status | STATUS_MAX_RT_MASK);
    if(status & STATUS_TX_DS_MASK)
        write_reg(STATUS, status | STATUS_TX_DS_MASK);
    return status;
}
