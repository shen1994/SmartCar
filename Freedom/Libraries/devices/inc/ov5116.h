#ifndef  _OV5116_H_
#define  _OV5116_H_
#include "common.h"
#include "gpio.h"
#include "Status.h"
#include "Image.h"
//场中断引脚 PA8
#define Vref_PORT   HW_GPIOB
#define Vref_PIN    2
//行中断引脚 PA9
#define Href_PORT   HW_GPIOA
#define Href_PIN    9
//信号引脚   PA10
#define Vedio_PORT   HW_GPIOD
#define Vedio_PIN    8

//中断
#define OV_VsyncIRQ   PORTB_IRQn
#define OV_HrefIRQ    PORTA_IRQn

#define HIGH_O     60
#define WIDTH_O    120

#define OVHrefDelay for(i=0;i<1;i++)
void ov5116_Init(void);
extern uint8_t ov5116_pic[HIGH_O][WIDTH_O];
//extern uint8_t Image_Flag;
#endif
