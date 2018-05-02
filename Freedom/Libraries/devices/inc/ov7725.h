#ifndef _OV7725_H_
#define _OV7725_H_

#include "stdlib.h"
#include "DMA.h"
#include "gpio.h"
#include "ov7725_reg.h"
#include "SCCB.h"

#ifdef _cplusplus
extern "c"{
#endif

typedef char (*OV7725_CallBackType)(char Sta);

#define IMAGE_FIFO_OK       1    //缓冲区获得数据
#define IMAGE_PROCESS_OK    0    //完成处理
#define IMAGE_PROCESS_NOK   1		 //没有完成处理

/*摄像头图片高宽*/
#define HIGH_D             60
#define WIDTH_D            80

#define OV7725_DATA_PORT       HW_GPIOD
#define OV7725_DATA_PORT_DMA   PTD
#define OV7725_DATA_OFFSET    (0) /* 摄像头数据引脚PTD0-PTD7 只能为 0 8 16 24 */

/*场中断,行中断*/
#define OV7725_VSYNC                      2                       //B2
#define OV7725_VSYNC_PORT                 HW_GPIOB
#define OV7725_VSYNC_IRQ                   PORTB_IRQn

#define OV7725_HREFC                       9                        //A9
#define OV7725_HREFC_PORT                  HW_GPIOA
#define OV7725_HREFC_IRQ                   PORTA_IRQn

/*PCLK*/
#define OV7725_PCLK_PORT                  HW_GPIOB                 //B3
#define OV7725_PCLK                       3
#define OV7725_PCLK_PORT_DMA  						PORTB_DMAREQ

/*DMA通道和中断*/
#define OV7725_DMA_CHANNAL     HW_DMA_CH1
#define OV7725_DMA_IRQ         DMA1_IRQn	//修改---原先DMA1_IRQn

//相关函数
Status Array_ImageDeal(uint8_t SrcImage[][(WIDTH_D / 8)],uint8_t Dst[][WIDTH_D]);
static void OV7725_delay_ms(unsigned int ms);
Status StartGetImage(void);
char OV7725_HREF_PROCESS(char Sta);
char OV7725_VREF_PROCESS(char Sta);
void OV7725_Href_CallbackInstall(OV7725_CallBackType AppCBFun);
void OV7725_Vref_CallbackInstall(OV7725_CallBackType AppCBFun);
void OV7725_VREF_ISR(uint32_t array);
void OV7725_HREF_ISR(uint32_t array);
void DMA_ISR(void);
Status OV7725_REG_Init(void);
Status OV7725_DMA_Init(void);
Status OV7725_Init(void);

extern uint8_t  OV7725_IMAGEB[HIGH_D][(WIDTH_D / 8)];
extern uint8_t Image_Flag;

#ifdef _cplusplus
}
#endif

#endif

