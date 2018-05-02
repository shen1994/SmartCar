/**
  ******************************************************************************
  * @file    spi_abstraction.h
  * @author  YANDLD
  * @version V2.5
  * @date    2013.12.25
  * @brief   www.beyondcore.net   http://upcmcu.taobao.com
  ******************************************************************************
  */

#ifndef __SPI_ABSTRACTION_H__
#define __SPI_ABSTRACTION_H__

#include <stdint.h>
#include <stdbool.h>

#define SPI_CPHA     (1<<0)                             /* bit[0]:CPHA, clock phase */
#define SPI_CPOL     (1<<1)                             /* bit[1]:CPOL, clock polarity */
/**
 * At CPOL=0 the base value of the clock is zero
 *  - For CPHA=0, data are captured on the clock's rising edge (low¡úhigh transition)
 *    and data are propagated on a falling edge (high¡úlow clock transition).
 *  - For CPHA=1, data are captured on the clock's falling edge and data are
 *    propagated on a rising edge.
 * At CPOL=1 the base value of the clock is one (inversion of CPOL=0)
 *  - For CPHA=0, data are captured on clock's falling edge and data are propagated
 *    on a rising edge.
 *  - For CPHA=1, data are captured on clock's rising edge and data are propagated
 *    on a falling edge.
 */

#define SPI_LSB      (0<<2)                             /* bit[2]: 0-LSB */
#define SPI_MSB      (1<<2)                             /* bit[2]: 1-MSB */

#define SPI_MASTER   (0<<3)                             /* SPI master device */
#define SPI_SLAVE    (1<<3)                             /* SPI slave device */

#define SPI_MODE_0       (0 | 0)                        /* CPOL = 0, CPHA = 0 */
#define SPI_MODE_1       (0 | SPI_CPHA)              /* CPOL = 0, CPHA = 1 */
#define SPI_MODE_2       (SPI_CPOL | 0)              /* CPOL = 1, CPHA = 0 */
#define SPI_MODE_3       (SPI_CPOL | SPI_CPHA)    /* CPOL = 1, CPHA = 1 */



#define SPI_EOK         (0x00)
#define SPI_ERROR       (0x01)


struct spi_config
{
    uint8_t mode;
    uint8_t data_width;
    uint32_t baudrate;
};

typedef enum
{
    kspi_cs_return_inactive,
    kspi_cs_keep_asserted,
} spi_cs_control_type;

typedef struct spi_device *spi_device_t;
struct spi_device
{
    uint32_t                    csn;            /* which pcs device has */
    struct spi_bus              *bus;           /* which bus device attacted to */
    struct spi_config           config;         /* device config attr */
};

typedef struct spi_bus *spi_bus_t;
struct spi_bus
{
    uint32_t instance;                          /* bus instance */
    const struct spi_ops *ops;                  /* bus ops */
    struct spi_device *owner;                   /* which device the bus currently owned */
};

struct spi_ops
{
    int (*configure)(spi_device_t device, struct spi_config *configuration);
    int (*read)(spi_device_t device, uint8_t *buf, uint32_t len, bool cs_return_inactive);
    int (*write)(spi_device_t device, uint8_t *buf, uint32_t len, bool cs_return_inactive);
};

static inline int spi_bus_register(struct spi_bus *bus, const struct spi_ops *ops)
{
    if(bus && ops)
    {
        bus->ops = ops;
        return SPI_EOK;
    }
    return SPI_ERROR;
}

static inline int spi_bus_attach_device(spi_bus_t bus, struct spi_device *device)
{
    if(bus && device)
    {
        bus->owner = device;
        device->bus = bus;
        return SPI_EOK;
    }
    return SPI_ERROR;
}

static inline int spi_read(spi_device_t device, uint8_t *buf, uint32_t len, bool cs_return_inactive)
{
    uint32_t ret;
    if(device != device->bus->owner)
    {
        device->bus->ops->configure(device, &device->config);
        device->bus->owner = device;
    }
    ret = device->bus->ops->read(device, buf, len, cs_return_inactive);
    return ret;
}

static inline int spi_write(spi_device_t device, uint8_t *buf, uint32_t len, bool cs_return_inactive)
{
    uint32_t ret;
    if(device != device->bus->owner)
    {
        device->bus->ops->configure(device, &device->config);
        device->bus->owner = device;
    }
    ret = device->bus->ops->write(device, buf, len, cs_return_inactive);
    return ret;
}

static inline int spi_config(spi_device_t device)
{
    return device->bus->ops->configure(device, &device->config);
}

//!< API functinos
int spi_bus_attach_device(spi_bus_t bus, struct spi_device *device);
int spi_bus_register(struct spi_bus *bus, const struct spi_ops *ops);
static inline int spi_config(spi_device_t device);
static inline int spi_read(spi_device_t device, uint8_t *buf, uint32_t len, bool cs_return_inactive);
static inline int spi_write(spi_device_t device, uint8_t *buf, uint32_t len, bool cs_return_inactive);

#endif

