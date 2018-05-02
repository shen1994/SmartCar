/****************************************Copyright (c)****************************************************
** File name:           PID.c
** Last modified Date:  2012-05-05
** Last Version:        V1.0
** Descriptions:        PID控制算法
*********************************************************************************************************/

#include "CusTomPID.h" 
#include "UartLink.h"

typedef struct PID{
    
    int   Point;                                                        /* 目标值                       */
    long  SumError;                                                     /* 累计误差                     */
    
    float P;                                                            /* 比例系数                     */
    float I;                                                            /* 积分系数                     */
    float D;                                                            /* 微分系数                     */
    
    int   LastError;                                                    /* Error[-1]                    */
    int   PrevError;                                                    /* Error[-2]                    */
                                                                        
}PID;

PID __PID;
PID *GoPID = &__PID;

/*********************************************************************************************************
** Function name:       PID_Init
** Descriptions:        PID所有参数清零
** input parameters:    *LPID:PID结构体指针                         
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/ 
void PID_Init(void) 
{
    GoPID->Point     = 0;

    GoPID->P         = 0;
    GoPID->I         = 0;
    GoPID->D         = 0;
    
    GoPID->SumError  = 0;    
    GoPID->LastError = 0;
    GoPID->PrevError = 0;
}

/*********************************************************************************************************
** Function name:       PID_SetPoint
** Descriptions:        设置PID调节的目标值
** input parameters:    AimValue:目标值                         
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/ 
void PID_SetPointGo(int AimValue) 
{
    GoPID->Point = AimValue;
}

/*********************************************************************************************************
** Function name:       PID_SetKp
** Descriptions:        设置P值
** input parameters:    Kp:                         
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/ 
void PID_SetKpGo(float Kp) 
{
    GoPID->P     = Kp; 
}
/*********************************************************************************************************
** Function name:       PID_SetKi
** Descriptions:        设置I值
** input parameters:    Ki:                         
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/ 
void PID_SetKiGo(float Ki) 
{
    GoPID->I     = Ki;
}
/*********************************************************************************************************
** Function name:       PID_SetKd
** Descriptions:        设置D值
** input parameters:    Kd:                         
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/ 
void PID_SetKdGo(float Kd) 
{
    GoPID->D     = Kd;
}
/*********************************************************************************************************
** Function name:       PID_SetKpid
** Descriptions:        设置D值
** input parameters:    Kd:                         
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/ 
void PID_SetKpidGo(float Kp,float Ki,float Kd) 
{
    GoPID->P     = Kp;
    GoPID->I     = Ki;
    GoPID->D     = Kd;
}
/*********************************************************************************************************
** Function name:       IncPID_Calculate
** Descriptions:        增量式PID控制
** input parameters:    BackValue:反馈值                         
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/ 
int IncPID_CalculateGo(int BackValue) 
{
    static int Error,Inc;
    Error   = GoPID->Point - BackValue;
    /********************************
    Δu(k)=u(k)-u(k-1)
          =Kp*(e(k)-e(k-1) + T/Ti*e(k) + Td/T*(e(k)  - 2*e(k-1)+e(k-2)))
          =Kp*(1+T/Ti+Td/T)*e(k) - Kp*(1+2*Td/T)*e(k-1) + Kp*Td/T*e(k-2))
					=Kp*(e(k)-e(k-1))+Kp*T/Ti*e(k)+Kp*Td/T*(e(k)-2*e(k-1)+e(k-2))
					=P * (e(k)-e(k-1)) + I * e(k) + D * (e(k)-2*e(k-1)+e(k-2));
    ***********************************/
		//简化计算
		//P提高响应时间，但是太大容易震荡，I消除震荡，D几乎作用很小，一般给很小
		///*
    Inc     = (int) ( GoPID->P * Error
              - GoPID->I * GoPID->LastError
              + GoPID->D * GoPID->PrevError);
		//*/
		//方便调试---计算量大
		/*
		int POut=0, DOut=0, IOut=0;
		POut = (int)(GoPID->P *(Error-GoPID->LastError));
		DOut = (int)(GoPID->I *Error);
		IOut = (int)(GoPID->D *(Error-2*GoPID->LastError+GoPID->PrevError));
		Inc = POut + DOut + IOut;
		*/
    GoPID->PrevError = GoPID->LastError;
    GoPID->LastError = Error;
    
    return Inc;
}
/*********************************************************************************************************
** Function name:       LocPID_Calculate
** Descriptions:        位置式PID控制
** input parameters:    BackValue:反馈值                         
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/ 
int LocPID_CalculateGo(int BackValue) 
{
    static int Error,delta,Loc;
    float PValue=0;
    Error           = GoPID->Point - BackValue;                          /* 计算当前误差Error[0]         */

		if(Error>300){
			PValue= 300*GoPID->P;
			Error=0;
		}
		else if(Error<-300){
			PValue= -300*GoPID->P;
			Error=0;
		}
		else{
			PValue= GoPID->P * Error;
		}
		/*变限积分*/
		if(Error>100)
		{
			Error=(300-Error)/200*Error;
		}
		else if(Error<-100)
		{
			Error=(300+Error)/200*Error;
		}
		
		GoPID->SumError+=Error;
		
		if(GoPID->SumError > 700) 
			GoPID->SumError = 700;
		if(GoPID->SumError < -700) 
			GoPID->SumError = -700;
		
    delta           = Error - GoPID->LastError;
		/********************************
    Δu(k)=u(k)-u(k-1)
          =Kp*(      e(k)-e(k-1) + T/Ti*e(k) +Td*(e(k)  - 2*e(k-1)+e(k-2))/T)
          =Kp*(1+T/Ti+Td/T)*e(k) - Kp*(1+2*Td/T)*e(k-1) + Kp*Td/T*e(k-2))
          =
    ***********************************/

    Loc     = (int) ( PValue
              + GoPID->I * GoPID->SumError
              + GoPID->D * delta);
							
    GoPID->LastError = Error;
    
    return Loc;
}

void  ClearI()
{
		GoPID->SumError=700;
}

void MoreClearI(){
	GoPID->SumError=400;
}
void  ClearI1()
{
		GoPID->SumError=-300;
}
int Shake_CalculateGo(int BackValue) 
{
    static int Error,delta,Loc;
   
    Error           = GoPID->Point - BackValue;                   
		if(Error>150)
			GoPID->SumError += 150;
		else if(Error<-150)
			GoPID->SumError -= 150;
		else
			GoPID->SumError+=Error;
		if(Error>150)
			Error=150;
		if(Error<-150)
			Error=-150;
    delta           = Error - GoPID->LastError;
    if(GoPID->SumError>700)
			GoPID->SumError =700;
		if(GoPID->SumError<-700)
			GoPID->SumError=-700;
    Loc     = (int) ( GoPID->P * Error
              + GoPID->I * GoPID->SumError
              + GoPID->D * delta);
    GoPID->LastError = Error;
		return 0;
}
