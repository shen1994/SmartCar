/****************************************Copyright (c)****************************************************
** File name:           PID.h
** Last modified Date:  2012-05-05
** Last Version:        V1.0u
** Descriptions:        PID.c的接口文件
*********************************************************************************************************/

#ifndef __CUSTOMPID_H__
#define __CUSTOMPID_H__
      
#include "common.h"

void PID_Init(void);
void PID_SetPointGo(int AimValue);
void PID_SetKpGo(float Kp);
void PID_SetKiGo(float Ki);
void PID_SetKdGo(float Kd);
void PID_SetKpidGo(float Kp,float Ki,float Kd);
int  IncPID_CalculateGo(int BackValue);
int  LocPID_CalculateGo(int BackValue);
int Shake_CalculateGo(int BackValue);
void  ClearI(void);
void  MoreClearI(void);
void  ClearI1(void);

#endif 
