/**
  ******************************************************************************
  * @file    i2c_abstraction.h
  * @author  YANDLD
  * @version V2.5
  * @date    2013.12.25
  * @brief   www.beyondcore.net   http://upcmcu.taobao.com
  ******************************************************************************
  */

#ifndef __I2C_ABSTRACTION_H__
#define __I2C_ABSTRACTION_H__

#include <stdint.h>



#define I2C_EOK         (0x00)
#define I2C_ERROR       (0x01)

struct i2c_config
{
    uint8_t mode;
    uint8_t data_width;
    uint32_t baudrate;
};

typedef struct i2c_device *i2c_device_t;
struct i2c_device
{
    uint8_t             chip_addr;        /*!< The slave's 7-bit address.*/
    uint32_t            subaddr;
    uint32_t            subaddr_len;
    struct i2c_bus      *bus;
    struct i2c_config   config;
};

struct i2c_ops
{
    int (*configure)(i2c_device_t device, struct i2c_config *configuration);
    int (*read)(i2c_device_t device, uint8_t *buf, uint32_t len);
    int (*write)(i2c_device_t device, uint8_t *buf, uint32_t len);
};

typedef struct i2c_bus *i2c_bus_t;
struct i2c_bus
{
    uint32_t                instance;
    const struct i2c_ops    *ops;
    struct i2c_device       *owner;
};

static inline int i2c_bus_register(struct i2c_bus *bus, const struct i2c_ops *ops)
{
    if(bus && ops)
    {
        bus->ops = ops;
        return I2C_EOK;
    }
    return I2C_ERROR;
}

static inline int i2c_bus_attach_device(i2c_bus_t bus, struct i2c_device *device)
{
    if(bus && device)
    {
        bus->owner = device;
        device->bus = bus;
        return I2C_EOK;
    }
    return I2C_ERROR;
}

static inline int i2c_config(i2c_device_t device)
{
    uint32_t ret;
    ret = device->bus->ops->configure(device, &device->config);
    return ret;
}

static inline int i2c_read(i2c_device_t device, uint8_t *buf, uint32_t len)
{
    uint32_t ret;
    if(device != device->bus->owner)
    {
        device->bus->ops->configure(device, &device->config);
        device->bus->owner = device;
    }
    ret = device->bus->ops->read(device, buf, len);
    return ret;
}

static inline int i2c_write(i2c_device_t device, uint8_t *buf, uint32_t len)
{
    uint32_t ret;
    if(device != device->bus->owner)
    {
        device->bus->ops->configure(device, &device->config);
        device->bus->owner = device;
    }
    ret = device->bus->ops->write(device, buf, len);
    return ret;
}

static inline int i2c_probe(i2c_device_t device)
{
    uint32_t ret;
    uint8_t dummy;
    ret = i2c_read(device, &dummy, 0);
    return ret;
}

//!< API functinos
int i2c_bus_attach_device(i2c_bus_t bus, struct i2c_device *device);
int i2c_bus_register(struct i2c_bus *bus, const struct i2c_ops *ops);
int i2c_config(i2c_device_t device);
int i2c_read(i2c_device_t device, uint8_t *buf, uint32_t len);
int i2c_write(i2c_device_t device, uint8_t *buf, uint32_t len);
int i2c_probe(i2c_device_t device);

#endif


