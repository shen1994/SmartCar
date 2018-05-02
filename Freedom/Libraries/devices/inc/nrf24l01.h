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
#ifndef __NRF24L01_H
#define __NRF24L01_H

#include <stdint.h>
#include "spi_abstraction.h"
#include "gpio.h"


#ifdef RAVEN
#define NRF24L01_CE_HIGH()     do {PAout(24) = 1;} while(0)
#define NRF24L01_CE_LOW()      do {PAout(24) = 0;} while(0)
#elif URANUS
#define NRF24L01_CE_HIGH()     do {PBout(0) = 1;} while(0)
#define NRF24L01_CE_LOW()      do {PBout(0) = 0;} while(0)
#elif ATOM
#define NRF24L01_CE_HIGH()     do {PAout(12) = 1;} while(0)
#define NRF24L01_CE_LOW()      do {PAout(12) = 0;} while(0)
#else
#warning  "no NRF24L01 CE operation defined!"
#define NRF24L01_CE_HIGH()     void(0)
#define NRF24L01_CE_LOW()      void(0)
#endif

#define  NRF_data_length   32

#define STATUS_RX_DR_MASK           0x40u
#define STATUS_TX_DS_MASK           0x20u
#define STATUS_MAX_RT_MASK          0x10u
#define STATUS              0x07

//<! API functions
void nrf24l01_set_tx_mode(void);
void nrf24l01_set_rx_mode(void);
int nrf24l01_write_packet(uint8_t *buf, uint32_t len);
int nrf24l01_read_packet(uint8_t *buf, uint32_t *len);
int nrf24l01_probe(void);
int nrf24l01_init(spi_bus_t bus, uint32_t cs);
void nrf24l01_FlushRX(void);
void nrf24l01_FlushTX(void);
char CheckStatus(void);
uint8_t read_reg(uint8_t addr);
void write_reg(uint8_t addr, uint8_t val);
#endif











