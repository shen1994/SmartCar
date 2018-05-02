/**
  ******************************************************************************
  * @file    spi_bus_ops.c
  * @author  YANDLD
  * @version V2.5
  * @date    2013.12.25
  * @brief   www.beyondcore.net   http://upcmcu.taobao.com
  * @note    此文件为芯片SPI总线通信集成驱动文件
  ******************************************************************************
  */
#include "spi_abstraction.h"
#include "spi.h"

//spi 通信配置
static int kinetis_spi_configure(spi_device_t device, struct spi_config *configuration)
{
    SPI_FrameFormat_Type mode;
    switch(configuration->mode & (SPI_CPHA | SPI_CPHA))
    {
    case SPI_MODE_0:
        mode = kSPI_CPOL0_CPHA0;
        break;
    case SPI_MODE_1:
        mode = kSPI_CPOL0_CPHA1;
        break;
    case SPI_MODE_2:
        mode = kSPI_CPOL1_CPHA0;
        break;
    case SPI_MODE_3:
        mode = kSPI_CPOL1_CPHA1;
        break;
    default:
        break;
    }
    SPI_FrameConfig(device->bus->instance, HW_CTAR0, mode, configuration->data_width, kSPI_MSBFirst, configuration->baudrate);
    return 0;
}
//spi总线读数据
static int kinetis_spi_read(spi_device_t device, uint8_t *buf, uint32_t len, bool cs_return_inactive)
{
    uint16_t dummy = 0xFFFF;
    len--;
    while(len--)
    {
        *buf++ = SPI_ReadWriteByte(device->bus->instance , HW_CTAR0, dummy, device->csn, kspi_cs_keep_asserted);
    }
    if(!cs_return_inactive)
    {
        *buf++ = SPI_ReadWriteByte(device->bus->instance, HW_CTAR0, dummy, device->csn, kspi_cs_keep_asserted);
    }
    else
    {
        *buf++ = SPI_ReadWriteByte(device->bus->instance, HW_CTAR0, dummy, device->csn, kspi_cs_return_inactive);
    }
    return SPI_EOK;
}
//spi写数据
static int kinetis_spi_write(spi_device_t device, uint8_t *buf, uint32_t len, bool cs_return_inactive)
{
    len--;
    while(len--)
    {
        SPI_ReadWriteByte(device->bus->instance, HW_CTAR0, *buf++, device->csn, kspi_cs_keep_asserted);
    }
    if(!cs_return_inactive)
    {
        SPI_ReadWriteByte(device->bus->instance, HW_CTAR0, *buf++, device->csn, kspi_cs_keep_asserted);
    }
    else
    {
        SPI_ReadWriteByte(device->bus->instance, HW_CTAR0, *buf++, device->csn, kspi_cs_return_inactive);
    }
    return SPI_EOK;
}

const static struct spi_ops kinetis_spi_ops =
{
    kinetis_spi_configure,
    kinetis_spi_read,
    kinetis_spi_write,
};

//spi通信总线初始化
int kinetis_spi_bus_init(struct spi_bus *bus, uint32_t instance)
{
    /* init hardware */
    SPI_InitTypeDef SPI_InitStruct1;
    SPI_InitStruct1.baudrate = 1000 * 1000;
    SPI_InitStruct1.frameFormat = kSPI_CPOL0_CPHA1;
    SPI_InitStruct1.dataSize = 8;
    SPI_InitStruct1.instance = instance;
    SPI_InitStruct1.mode = kSPI_Master;
    SPI_InitStruct1.bitOrder = kSPI_MSBFirst;
    SPI_InitStruct1.ctar = HW_CTAR0;
    SPI_Init(&SPI_InitStruct1);
    /* register bus */
    bus->instance = instance;
    return spi_bus_register(bus, &kinetis_spi_ops);
}

