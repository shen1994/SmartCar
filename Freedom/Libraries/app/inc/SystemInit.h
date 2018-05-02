#ifndef _SYSTEM_INIT_H
#define  _SYSTEM_INIT_H

#include "app_include.h"
#include "Control.h"
#include "Status.h"

#define USE_OLED
#define USE_OV7725
#define USE_BUZZER
#define USE_USART3
#define USE_MOTOR
#define USE_FIVE_KEY
//#define USE_PIT0
#define USE_PID
#define USE_FUZZY

Status DelaySysTime(void);
Status System_Init(void);
static void Uart_ISR(uint16_t DataRec);
Status ImageCapture(void);
void PIT_DoCallBack(void);
extern uint8_t isCamera;
extern uint8_t isRun;
extern uint8_t BeepLoopTime;
#endif
