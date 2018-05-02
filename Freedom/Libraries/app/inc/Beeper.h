#ifndef  _BPPER_H
#define _BPPER_H
#include "gpio.h"
#include "Status.h"
#define Buzzer_Init GPIO_QuickInit(HW_GPIOE,28,kGPIO_Mode_OPP)
#define Buzzer_Off  GPIO_WriteBit(HW_GPIOE,28,0)
#define Buzzer_On   GPIO_WriteBit(HW_GPIOE,28,1)
Status Beeper(char *Count,char Continous,char Rest);
Status BeeperLoopTime(unsigned char* LoopTime);
#endif 
