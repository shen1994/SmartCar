#ifndef _SCCB_H
#define _SCCB_H

#include "gpio.h"
#include "Status.h"

#define ADDR_OV7725   0x42
#define ADDR_OV7620   0x42
#define ADDR_MT9V022  0x90
#define DEV_ADDR ADDR_OV7725

#define SCCB_PORT    HW_GPIOB
#define SCCB_SCK     17
#define SCCB_SDA     10

#define SCCB_SCK_H()     GPIO_WriteBit(SCCB_PORT, SCCB_SCK, 1)
#define SCCB_SCK_L()     GPIO_WriteBit(SCCB_PORT, SCCB_SCK, 0)

#define SCCB_SDA_H()     GPIO_WriteBit(SCCB_PORT, SCCB_SDA, 1)
#define SCCB_SDA_L()     GPIO_WriteBit(SCCB_PORT, SCCB_SDA, 0)
/**/
#define SCCB_SDA_IN      SCCB_SDA_INPUT()
#define SCCB_SDA_OUT     SCCB_SDA_OUTPUT()

//#define SCCB_DELAY       SCCB_delay_us(1500)

#define SCCB_SDA_STATE	 GPIO_ReadBit(SCCB_PORT, SCCB_SDA)


Status SCCB_GPIO_Config(void);
Status SCCB_SDA_OUTPUT(void);
Status SCCB_SDA_INPUT(void);
Status startSCCB(void);
Status stopSCCB(void);
Status noAck(void);
Status Ack(void);
unsigned char SCCBwriteByte(unsigned char m_data);
unsigned char SCCBreadByte(void);
Status SCCB_WriteDoubleRegister(unsigned char regID, unsigned char regDat);
Status SCCB_ReadRegister(unsigned char* pBuffer, unsigned int Length, unsigned char regID);

#endif


