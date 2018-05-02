/****************************************Copyright (c)****************************************************
** File name:           PID.h
** Last modified Date:  2012-05-05
** Last Version:        V1.0
** Descriptions:        PID.c的接口文件
*********************************************************************************************************/

#ifndef __PID_H__
#define __PID_H__
      
#include "common.h"

void PID_Init(void);
void PID_SetPoint(int AimValue);
void PID_SetKp(float Kp);
void PID_SetKi(float Ki);
void PID_SetKd(float Kd);
void PID_SetKpid(float Kp,float Ki,float Kd);
int  IncPID_Calculate(int BackValue);
int  LocPID_Calculate(int BackValue);

#endif 
