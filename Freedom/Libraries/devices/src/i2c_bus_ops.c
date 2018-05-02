/**
  ******************************************************************************
  * @file    i2c_bus_ops.c
  * @author  YANDLD
  * @version V2.5
  * @date    2013.12.25
  * @brief   www.beyondcore.net   http://upcmcu.taobao.com
  ******************************************************************************
  */

#include "i2c_abstraction.h"
#include "i2c.h"

/**
 * @brief  i2c configuration ops for Kinetis device
 */
static int kinetis_i2c_configure(i2c_device_t device, struct i2c_config *configuration)
{
#ifdef LIB_DEBUG
    printf("i2c config not supported!\r\n");
#endif
    return I2C_EOK;
}

/**
 * @brief  i2c_read ops for Kinetis device
 */
static int kinetis_i2c_read (i2c_device_t device, uint8_t *buf, uint32_t len)
{
    int ret = I2C_BurstRead(device->bus->instance, device->chip_addr, device->subaddr, device->subaddr_len, buf, len);
    if(ret)
    {
        return I2C_ERROR;
    }
    return I2C_EOK;
}

/**
 * @brief  i2c write ops for Kinetis device
 */
static int kinetis_i2c_write (i2c_device_t device, uint8_t *buf, uint32_t len)
{
    int ret = I2C_BurstWrite(device->bus->instance, device->chip_addr, device->subaddr, device->subaddr_len, buf, len);
    if(ret)
    {
        return I2C_ERROR;
    }
    return I2C_EOK;
}

const static struct i2c_ops kinetis_i2c_ops =
{
    kinetis_i2c_configure,
    kinetis_i2c_read,
    kinetis_i2c_write,
};

int kinetis_i2c_bus_init(struct i2c_bus *bus, uint32_t instance)
{
    /* init hardware */
    I2C_InitTypeDef I2C_InitStruct1;
    I2C_InitStruct1.baudrate = 100 * 1000;
    I2C_InitStruct1.instance = instance;
    I2C_Init(&I2C_InitStruct1);
    /* register bus */
    bus->instance = instance;
    return i2c_bus_register(bus, &kinetis_i2c_ops);
}

