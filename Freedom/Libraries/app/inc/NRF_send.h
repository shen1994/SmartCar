#ifndef __NRF_SEND_H
#define __NRF_SEND_H
#include <stdint.h>
#include "gpio.h"
#include "nrf24l01.h"
#include "spi_abstraction.h"
#include "spi.h"
#include "uart.h"
#include "Auto_Binaryzation.h"
#include "MT9V022.h"
#include "PC_LINK.h"
#include "app_include.h"
#define NRF_irq_port HW_GPIOE
#define NRF_irq_pin  28

typedef Status (*NRF_CallBackType)(uint8_t *, uint32_t);

Status NRF_Gpio_Init(void);
Status NRF_CallbackInstall(NRF_CallBackType AppCBFun);
Status Data_Rec_Analyse(uint8_t *rxbuff, uint32_t len);
Status NRF_Send_Image(uint8_t image[][180], int HIGH, int WIDTH);

void NRF_ISR(uint32_t array);
extern  uint8_t NRF2401RXBuffer[NRF_data_length];

#endif
